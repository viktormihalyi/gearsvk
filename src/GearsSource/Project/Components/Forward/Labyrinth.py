import Gears as gears
from .. import * 
from PyQt5.QtCore import Qt

try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
except:
  print ('ERROR: PyOpenGL not installed properly.')

import random

class KohenekBartels() :

    def __init__(self, controlPoints, loopTime):
        self.knots = []
        for cp in controlPoints:
            self.knots += [cp[0]]
        loopCp = (loopTime, controlPoints[0][1], controlPoints[0][2], controlPoints[0][3], controlPoints[0][4], controlPoints[0][5])
        cpx = [controlPoints[-1]] + controlPoints + [loopCp]
        self.vs = []
        for cpp, cp, cpn in zip(cpx, cpx[1:], cpx[2:]) :
            self.vs += [(
                    ((cpn[1] - cp[1]) / (cpn[0] - cp[0]) * cp[5] + (cp[1] - cpp[1] ) / (cp[0] - cpp[0]) * (1 - cp[5])) * (1 - cp[4]),
                    ((cpn[2] - cp[2]) / (cpn[0] - cp[0]) * cp[5] + (cp[2] - cpp[2] ) / (cp[0] - cpp[0]) * (1 - cp[5])) * (1 - cp[4]),
                    ((cpn[3] - cp[3]) / (cpn[0] - cp[0]) * cp[5] + (cp[3] - cpp[3] ) / (cp[0] - cpp[0]) * (1 - cp[5])) * (1 - cp[4])
                    )]
        self.vs += [self.vs[0]]
        self.knots += [loopTime]
        self.abcd = []
        for x0, x1, v0, v1 in zip(cpx[1:], cpx[2:], self.vs, self.vs[1:]) :
            self.abcd += [(
                    (  -2 * (x1[1] - x0[1]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) +
                           (v1[0] + v0[0]) / (x1[0] - x0[0]) / (x1[0] - x0[0]),
                       -2 * (x1[2] - x0[2]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) +
                           (v1[1] + v0[1]) / (x1[0] - x0[0]) / (x1[0] - x0[0]),
                       -2 * (x1[3] - x0[3]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) +
                           (v1[2] + v0[2]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) ),
                    (   3 * (x1[1] - x0[1]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) -
                            (v1[0] + 2*v0[0]) / (x1[0] - x0[0]),
                        3 * (x1[2] - x0[2]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) -
                            (v1[1] + 2*v0[1]) / (x1[0] - x0[0]),
                        3 * (x1[3] - x0[3]) / (x1[0] - x0[0]) / (x1[0] - x0[0]) -
                            (v1[2] + 2*v0[2]) / (x1[0] - x0[0])),
                    (v0[0], v0[1], v0[2]),
                    (x0[1], x0[2], x0[3])
                    )]
        print(self.vs)
        print(self.abcd)

    def fmod(self, x, y):
        return x - y * math.floor(x/y)

    def get(self, t):
        t = self.fmod(t, self.knots[-1])
        prevt = 0
        for ti, abcd in zip(self.knots[1:], self.abcd):
            if ti > t:
                t -= prevt
                #print(t)
                #print(abcd)
                return [(abcd[0][0] * t * t * t +  abcd[1][0] * t * t + abcd[2][0] * t + abcd[3][0],
                     abcd[0][1] * t * t * t +  abcd[1][1] * t * t + abcd[2][1] * t + abcd[3][1],
                     abcd[0][2] * t * t * t +  abcd[1][2] * t * t + abcd[2][2] * t + abcd[3][2] ),
                     (3 * abcd[0][0] * t * t +  2 * abcd[1][0] * t + abcd[2][0] ,
                     3 * abcd[0][1] * t * t +  2 * abcd[1][1] * t + abcd[2][1] ,
                     3 * abcd[0][2] * t * t +  2 * abcd[1][2] * t + abcd[2][2] ) ]
            prevt = ti

def box() :
    glColor3f(1.0,1.0,1.0)
    glBegin(GL_QUADS)
    #glColor3f(0.0,1.0,0.0)
    glTexCoord2f(1.0, 0.0)
    glVertex3f(1.0, 1.0,-1.0)
    glTexCoord2f(0.0, 0.0)
    glVertex3f(-1.0, 1.0,-1.0)
    glTexCoord2f(0.0, 1.0)
    glVertex3f(-1.0, 1.0, 1.0)
    glTexCoord2f(1.0, 1.0)
    glVertex3f(1.0, 1.0, 1.0)
    #glColor3f(1.0,0.5,0.0)
    glTexCoord2f(1.0, 1.0)
    glVertex3f(1.0,-1.0, 1.0)
    glTexCoord2f(0.0, 1.0)
    glVertex3f(-1.0,-1.0, 1.0)
    glTexCoord2f(0.0, 0.0)
    glVertex3f(-1.0,-1.0,-1.0)
    glTexCoord2f(1.0, 0.0)
    glVertex3f(1.0,-1.0,-1.0)
    #glColor3f(1.0,0.0,0.0)
    glTexCoord2f(1.0, 1.0)
    glVertex3f(1.0, 1.0, 1.0)
    glTexCoord2f(0.0, 1.0)
    glVertex3f(-1.0, 1.0, 1.0)
    glTexCoord2f(0.0, 0.0)
    glVertex3f(-1.0,-1.0, 1.0)
    glTexCoord2f(1.0, 0.0)
    glVertex3f(1.0,-1.0, 1.0)
    #glColor3f(1.0,1.0,0.0)
    glTexCoord2f(1.0, 0.0)
    glVertex3f(1.0,-1.0,-1.0)
    glTexCoord2f(0.0, 0.0)
    glVertex3f(-1.0,-1.0,-1.0)
    glTexCoord2f(0.0, 1.0)
    glVertex3f(-1.0, 1.0,-1.0)
    glTexCoord2f(1.0, 1.0)
    glVertex3f(1.0, 1.0,-1.0)
    #glColor3f(0.0,0.0,1.0)
    glTexCoord2f(1.0, 1.0)
    glVertex3f(-1.0, 1.0, 1.0)
    glTexCoord2f(1.0, 0.0)
    glVertex3f(-1.0, 1.0,-1.0)
    glTexCoord2f(0.0, 0.0)
    glVertex3f(-1.0,-1.0,-1.0)
    glTexCoord2f(0.0, 1.0)
    glVertex3f(-1.0,-1.0, 1.0)
    #glColor3f(1.0,0.0,1.0)
    glTexCoord2f(1.0, 0.0)
    glVertex3f(1.0, 1.0,-1.0)
    glTexCoord2f(1.0, 1.0)
    glVertex3f(1.0, 1.0, 1.0)
    glTexCoord2f(0.0, 1.0)
    glVertex3f(1.0,-1.0, 1.0)
    glTexCoord2f(0.0, 0.0)
    glVertex3f(1.0,-1.0,-1.0)
    glEnd()

class Labyrinth() : 
    args = None

    def __init__(self, **args):
        self.args = args

    def apply(self, stimulus) :
        self.applyWithArgs(stimulus, **self.args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            controlPoints   :   'list of Kohanek-Bertels control points for camera movement. Every entry is a tuple (t, x, y, z, tension, bias).'
                            #=   [(0, 10, 0.5, 10, 0.9, 0.5), (120, 22, 0.5, 22, 0.9, 0.5), (240, 10, 0.5, 22, 0.9, 0.5)],
                            #=   [(0, 10, 0.5, 10, 0, 0.5), (180, 22, 0.5, 22, 0, 0.5)],
                            =   [(0, 10, 2.5, 10, 0, 0.5), (20, 20, 2.5, 10, 0, 0.5), (240, 20, 2.5, 20, 0, 0.5)],
            loopTime        :   ''
                            =   360,
            minSpeed        =   -1,
            maxSpeed        =   1,
            grid            :   ''
                            =   
"""XXXXXXXXXXXXXXXX
X              X
X   X X X      X
X  X     X     X
X   X  X     XXX
XX             X
X      X       X
X     X X      X
XX            XX
XX             X
X  X         X X
X       X  XX  X
X X   X        X
X           X  X
X       X      X
XXXXXXXXXXXXXXXX"""
            ) :
        self.kbSpline = KohenekBartels(controlPoints, loopTime)
        self.grid = grid

        stimulus.enableColorMode()
        gears.loadTexture('./Project/Media/tron.jpg')
        #gears.loadTexture('./Project/Media/leopard.png')

        stimulus.setForwardRenderingCallback(self.render)
        stimulus.registerCallback(gears.StimulusStartEvent.typeId, self.start)
        stimulus.registerCallback(gears.StimulusEndEvent.typeId, self.finish )

        stimulus.registerCallback(gears.WheelEvent.typeId, self.onMouseWheel )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        self.time = 0
        self.speed = 0.01
        self.minSpeed = minSpeed
        self.maxSpeed = maxSpeed
        self.prevX = 0
        self.prevY = 0

    def start( self, event ):
        self.glist = glGenLists(1)
        glNewList(self.glist, GL_COMPILE)
        glBegin(GL_QUADS)
        glColor3f(1.0, 1.0, 1.0)
        glTexCoord2f(0, 0)
        glVertex3f(-1.0, 0.0, -1.0)
        glTexCoord2f(0, 16)
        glVertex3f(31.0, 0.0, -1.0)
        glTexCoord2f(16, 16)
        glVertex3f(31.0, 0.0, 31.0)
        glTexCoord2f(0, 16)
        glVertex3f(-1.0, 0.0, 31.0)
        glEnd()
        glPushMatrix()
        glScaled(1, 2, 1)
        glTranslated(0, 1, 0)
        for i in self.grid :
            glTranslated(2, 0, 0)
            if i == 'X':
                box()
            if i == '\n':
                glTranslated(-34, 0, 2)
        glPopMatrix()
        glEndList()

    def finish( self, event ):
        glDeleteLists(self.glist, 1)

    def render(self, iFrame):
        #TODO handle skipped frames
        self.time += self.speed
        #self.time = 0.1 * iFrame

        glEnable(GL_DEPTH_TEST)
        glDepthMask(GL_TRUE)
        glClearColor(0.0, 0, 0.0, 1.0 )
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45, 1, 0.1, 1000)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        pos, vel = self.kbSpline.get(self.time)
        #gluLookAt(16, 0.5, 20- iFrame * 0.1, 16, 0.5, 19 - iFrame * 0.1, 0, 1, 0)
        #print(pos)
        gluLookAt(pos[0], pos[1], pos[2], pos[0]+vel[0], pos[1]+vel[1], pos[2]+vel[2], 0, 1, 0)
        #print("{t}: {x}, {y}, {z}".format( t=iFrame, x=pos[0], y=pos[1], z=pos[2]))
        glEnable(GL_TEXTURE_2D)
        glActiveTexture(GL_TEXTURE0) 
        #gears.bindTexture('./Project/Media/leopard.png')
        gears.bindTexture('./Project/Media/tron.jpg')
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
        glCallList(self.glist)
        glDisable(GL_DEPTH_TEST)
        glDepthMask(GL_FALSE);

    def onMouseWheel(self, event) :
        deltaY = event.deltaY()
        self.speed += deltaY * 0.01
        if self.speed < self.minSpeed:
            self.speed = self.minSpeed
        if self.speed > self.maxSpeed:
            self.speed = self.maxSpeed

    def onKey(self, event):
        if event.text() == 'O':
            self.speed = 0