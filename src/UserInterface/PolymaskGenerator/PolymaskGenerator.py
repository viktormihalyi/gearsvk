from PyQt5.QtGui import (QColor)
from PyQt5.QtWidgets import (QErrorMessage)
import GearsUtils as utils
import numpy as np
from math import sqrt
#from Gears import CDT
from Gears import loadTexture
from enum import Enum

PASSTHROUGH_VERTEX = '''
    #version 330

    in vec4 vPosition;

    out vec2 fTexCoord;

    void main(void) {
        gl_Position = vPosition;
        fTexCoord = (vec2(vPosition.x, -vPosition.y) + vec2(1.f, 1.f)) * 0.5f;
    }
'''

SIMPLE_FRAGMENT = '''
    #version 330

    uniform sampler2D tex;
    in vec2 fTexCoord;
    out vec4 outColor;

    void main() {
        outColor = vec4(texture(tex,fTexCoord));
    }
'''

QUAD = [-1.0,  1.0,
		 1.0,  1.0,
		-1.0, -1.0,
		-1.0, -1.0,
		 1.0,  1.0,
		 1.0, -1.0]

def get_lines_intersection(P1, P2, P3, P4):
    a1 = P2[1] - P1[1]
    b1 = P1[0] - P2[0]
    d1 = (P1[0] - P2[0]) * P1[1] - (P1[1] - P2[1]) * P1[0]

    a2 = P4[1] - P3[1]
    b2 = P3[0] - P4[0]
    d2 = (P3[0] - P4[0]) * P3[1] - (P3[1] - P4[1]) * P3[0]

    # x = (b2d1 - b1d2)/(a1b2 - a2b1)
    # y = (a1d2 - a2d1)/(a1b2 - a2b1)

    nx = (a1 * b2 - a2 * b1)
    ny = (a1 * b2 - a2 * b1)
    if nx == 0 or ny == 0: # check 0 division
        return None

    x = (b2 * d1 - b1 * d2) / nx
    y = (a1 * d2 - a2 * d1) / ny
    
    return [x,y]

def check_point_on_section(P, P1, P2):
    if P1 == P or P2 == P: # intersection at the end
        return True
    if P1[0] == P[0] or P2[0] == P1[0]: # do not devide with zero
        return False
    m1 = P2[1] - P1[1] / (P2[0] - P1[0])
    m2 = P[1] - P1[1] / (P[0] - P1[0])

    if (m1 < 0 and m2 > 0) or (m1 > 0 and m2 < 0):
        return False
    
    return sqrt((P[0] - P1[0]) * (P[0] - P1[0]) + (P[1] - P1[1]) * (P[1] - P1[1])) < sqrt((P2[0] - P1[0]) * (P2[0] - P1[0]) + (P2[1] - P1[1]) * (P2[1] - P1[1]))

class PolymaskChangeEvent(Enum):
    SelectionChanged = 0
    ItemChanged = 1
    ItemAdded = 2
    ItemRemoved = 3

CATMULL_ROM_MTX = np.array([[0,    1,    0,    0],
            [-0.5,    0,  0.5,    0],
            [1, -2.5,    2, -0.5],
            [-0.5,  1.5, -1.5,  0.5]])

class PolymaskGenerator(QGLWidget):
    def __init__(self, parent, winId, splines, dataChanged):
        super().__init__()

        self.selectedControlIndex = None
        self.last_selected = 0
        self.selected_spline = 0
        self.dataChanged = dataChanged
        self.splines = splines
        self.background = None

        utils.initQGLWidget(self, super(), parent, winId)

        color = QColor(255, 255, 255, 255)
        self.qglClearColor(color)

        self.splineColor = (0.0,0.0,0.0)
        self.selectedSplineColor = (0.0,0.0,1.0)
        self.CPColor = (1.0,0.0,0.0)
        self.selectedCPColor = (0.0,0.0,1.0)

        self.fill = {}

        vertex_shader = shaders.compileShader(PASSTHROUGH_VERTEX, GL_VERTEX_SHADER)
        fragment_shader = shaders.compileShader(SIMPLE_FRAGMENT, GL_FRAGMENT_SHADER)

        self.shader = shaders.compileProgram(vertex_shader, fragment_shader)


    def resizeGL(self, w, h):
        self.setFixedSize(1280, 720)
        glViewport(0, 0, 1280, 720)

    def paintGL(self):
        glClear(GL_COLOR_BUFFER_BIT)
        glEnableClientState(GL_VERTEX_ARRAY)
        if self.background != None:
            self.drawBackground()
        if len(self.splines) > 0: 
            self.drawCurves()
            self.drawControlPoints()
        glDisableClientState(GL_VERTEX_ARRAY)

    def drawBackground(self):
        shaders.glUseProgram(self.shader)
        glEnable(GL_TEXTURE_2D)
        varid = glGetUniformLocation(self.shader, "tex")
        texid = loadTexture(self.background)
        if texid >= 0:
            glActiveTexture(GL_TEXTURE0 + texid)
            glBindTexture(GL_TEXTURE_2D, texid)
            glUniform1i(varid, texid)

        glVertexPointer(2, GL_FLOAT, 0, QUAD)
        glDrawArrays(GL_TRIANGLES, 0, 6)

        glDisable(GL_TEXTURE_2D)
        shaders.glUseProgram(0)

    def drawControlPoints(self):
        cp_list = [ c for vec in [i.tolist() for i in [s for sp in self.splines for s in sp]] for c in vec ]
        glVertexPointer(2, GL_FLOAT, 0, cp_list)
        glColor3f(*(self.CPColor))
        glPointSize(10)
        glDrawArrays(GL_POINTS, 0, int(len(cp_list) / 2))
        glColor3f(*(self.selectedCPColor))
        current_idx = 0
        for i in range(self.selected_spline):
            current_idx += len(self.splines[i])

        current_idx += self.last_selected
        if current_idx >= 0:
            glDrawArrays(GL_POINTS, current_idx, 1)

    def mousePressEvent(self, e):
        mouse = [(e.x() - self.width() / 2) * 2, (-e.y() + self.height() / 2) * 2]

        self.selectedControlIndex = None
        for s_idx in range(len(self.splines)):
            for idx, point in enumerate(self.splines[s_idx]):
                p = point.tolist()
                if np.linalg.norm(np.array([p[0] * self.width(), p[1] * self.height()]) - np.array(mouse)) < 10:
                    self.selectedControlIndex = idx
                    self.last_selected = idx
                    self.selected_spline = s_idx
                    break

        if self.selectedControlIndex != None:
            self.onDataChanged(PolymaskChangeEvent.SelectionChanged, self.selectedControlIndex)

    def mouseReleaseEvent(self, e):
        self.selectedControlIndex = None

    def mouseMoveEvent(self, e):
        if self.selectedControlIndex == None:
            return
        mouse = [e.x() / self.width(), (self.height() - e.y()) / self.height()]
        mouse = [min(max((i - 0.5) * 2, -1.0), 1.0) for i in mouse]

        if not next((True for item in self.splines[self.selected_spline] if np.array_equal(item, np.array(mouse))), False):
            self.splines[self.selected_spline][self.selectedControlIndex] = np.array(mouse)
            self.onDataChanged(PolymaskChangeEvent.ItemChanged, self.selectedControlIndex, mouse)
            self.update()

    def drawCurves(self):
        self.vertecies = []

        glColor3f(0.8, 0.8, 0.8)
        for s_idx in range(len(self.splines)):
            if s_idx in self.fill:
                glVertexPointer(2, GL_FLOAT, 0, self.fill[s_idx])
                glDrawArrays(GL_TRIANGLES, 0, int(len(self.fill[s_idx]) / 2))

        glLineWidth(2.0)
        glColor3f(*(self.splineColor))
        for s in self.splines:
            self.vertecies.append(self.drawCurve(s))

        glColor3f(*(self.selectedSplineColor))
        self.drawCurve(self.splines[self.selected_spline])

    def drawCurve(self, spline):
        current_vertecies = []
        for idx, val in enumerate(spline):
            i0 = idx - 1
            i2 = idx + 1
            i3 = idx + 2
            if i2 >= len(spline):
                i2 = 0
                i3 = 1
            elif i3 >= len(spline):
                i3 = 0

            current_vertecies.extend(self.drawSegment(np.array([spline[i0],
                val,
                spline[i2],
                spline[i3]])))
        current_vertecies.extend(spline[0].tolist())

        glVertexPointer(2, GL_FLOAT, 0, current_vertecies)
        glDrawArrays(GL_LINE_STRIP, 0, int(len(current_vertecies) / 2))

        return current_vertecies

    def drawSegment(self, P, precision=50):
        vertecies = []
        vertecies.extend(P[1].tolist())
        t = 1 / precision
        while t < 0.99999999999999:
            catmull_rom_vec = np.matmul(np.array([1, t, t * t, t * t * t]), CATMULL_ROM_MTX)

            ft = np.dot(catmull_rom_vec, P)
            vertecies.extend([float(i) for i in ft.tolist()])
            t += 1 / precision

        return vertecies

    def addBefore(self, index):
        before = index - 1
        if before < 0:
            before = len(self.splines[self.selected_spline]) - 1

        new_point = (self.splines[self.selected_spline][before] + self.splines[self.selected_spline][index]) / 2
        self.splines[self.selected_spline].insert(index, new_point)
        self.onDataChanged(PolymaskChangeEvent.ItemAdded, index, new_point.tolist())
        self.update()

    def addAfter(self, index):
        after = index + 1
        if after >= len(self.splines[self.selected_spline]):
            after = 0

        new_point = (self.splines[self.selected_spline][after] + self.splines[self.selected_spline][index]) / 2
        self.splines[self.selected_spline].insert(index + 1, new_point)
        self.onDataChanged(PolymaskChangeEvent.ItemAdded, index + 1, new_point.tolist())
        self.update()

    def removeCurrent(self, index):
        if len(self.splines[self.selected_spline]) == 3:
            box = QErrorMessage(self)
            box.showMessage("A curve must have minimum 3 control points!")
            return

        self.splines[self.selected_spline].pop(index)
        self.onDataChanged(PolymaskChangeEvent.ItemRemoved, index)

    def generateTriangles(self):
        self.fill = {}
        for idx, vert in enumerate(self.vertecies):
            if self.checkIntersection(vert):
                box = QErrorMessage(self)
                box.showMessage("Intersection in one curve is not supported!")
                return False
            #cdt = CDT(vert[:-2]) # do not add last point again, poly2tri #cdt
            #= CDT(vert[:-2]) # do not add last point again, poly2tri
            #crashes if there is the same point
            #crashes if there is the same point
                                 # twice
            #cdt.triangulate()
            #self.fill[idx] = cdt.get_triangles()
        self.update()
        return True

    def checkIntersection(self, p):
        i = 0
        size = len(p)
        # to_insert = {}
        # to_insert_keys = []
        while i < size - 6:
            j = i + 4
            while j < size - 2:
                if i == 0 and j == size - 4: # first segment is next to the last segment
                    j += 2
                    continue
                P1 = [p[i], p[i + 1]]
                P2 = [p[i + 2], p[i + 3]]
                P3 = [p[j], p[j + 1]]
                P4 = [p[j + 2], p[j + 3]]
                I = get_lines_intersection(P1, P2, P3, P4)
                if I != None:
                    if check_point_on_section(I, P1, P2) and check_point_on_section(I, P3, P4):
                        return True
                        #xto_insert[i] = I
                        #to_insert_keys.append(i)
                j += 2
            i += 2

        # to_insert_keys.sort(reverse=True)
        # for i in to_insert_keys:
        #     self.vertecies.insert(i, to_insert[i][1])
        #     self.vertecies.insert(i, to_insert[i][0])
        return False

    def onDataChanged(self, type, index, value=None):
        self.fill = {}
        data = {
            "s_idx": self.selected_spline,
            "idx": index,
            "value": value
        }
        self.dataChanged(type, data)

    def currentPointChanged(self, idx):
        self.last_selected = idx
        self.update()

    def addSpline(self, points):
        self.splines.append([np.array(points[0]),
            np.array(points[1]),
            np.array(points[2]),
            np.array(points[3])])
        self.update()

    def removeSpline(self, index, new_index):
        self.splines.pop(index)
        self.selected_spline = new_index
        self.update()

    def currentSplineChanged(self, index):
        self.selected_spline = index
        self.update()

    def setBackground(self, file):
        if len(file) > 0:
            self.background = file
            self.update()

    def setSplineColor(self, obj, r, g, b):
        setattr(self, obj, (r / 255, g / 255, b / 255))