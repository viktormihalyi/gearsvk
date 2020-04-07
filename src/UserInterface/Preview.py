import sys
import Gears as gears
import importlib.machinery
import os
import GearsUtils as utils
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer, QSize)
from PyQt5.QtWidgets import (QWidget, QToolTip, QPushButton, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout, QSizePolicy)
from PyQt5.QtGui import (QFont, QPalette, QFontMetrics, QOpenGLContext, QPainter )
from PyQt5.QtOpenGL import (QGLWidget, QGLFormat, QGLContext)
try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
except:
  print ('ERROR: PyOpenGL not installed properly.')


class Preview(QGLWidget):

    def __init__(self, parent, editor, winId):
        utils.initQGLWidget(self, super(), parent, winId)
        self.sFrame = 0
        self.editor = editor
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

    def initializeGL(self):
        err = glGetError()
        if(err):
            print("An OpenGL error occcured in PyQt. A known cause for this is a driver problem with Intel HD graphics chipsets. Try updating your driver, manually if necessary.")
            print("OpenGL error code: " + str(err))

    def resizeGL(self, w, h):
        self.width = w
        self.height = h
        glViewport(0, 0, w, h)

    def paintGL(self):
        stim = gears.renderSample(self.sFrame, 0, 0, self.width, self.height)
        if not stim:
           self.sFrame = 0
        else:
            self.editor.indicatePreviewProgress(stim.tb[-1][1])

    def sizeHint(self):
        s = self.size()
        self.lastWidth = s.width()
        s.setHeight(s.width() / 16 * 9 )
        return s

    def resizeEvent(self, event):
        r = super().resizeEvent(event)
        if self.width != self.lastWidth :
            self.updateGeometry()
        return r

    def minimumSizeHint(self):
        return QSize(160, 90)