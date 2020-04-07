import sys
import Gears as gears
import importlib.machinery
import os
import GearsUtils as utils
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer, QSize)
from PyQt5.QtWidgets import (QWidget, QToolTip, QPushButton, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout)
from PyQt5.QtGui import (QFont, QPalette )
from PyQt5.QtOpenGL import (QGLWidget, QGLFormat, QGLContext)
try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
except:
  print ('ERROR: PyOpenGL not installed properly.')


class TemporalKernelPlot(QGLWidget):
    width = 128
    height = 128

    def __init__(self, parent, launcher, winId):
        utils.initQGLWidget(self, super(), parent, winId)
        self.launcher = launcher

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
        gears.drawTemporalKernel()

    def minimumSizeHint(self):
        return QSize(800, 256)
