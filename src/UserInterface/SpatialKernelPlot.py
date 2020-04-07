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


class SpatialKernelPlot(QGLWidget):


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
        if(h > w):
            glViewport((h-w)//2, 0, w, w)
        else:
            glViewport((w-h)//2, 0, h, h)
            

    def paintGL(self):
        gears.drawSpatialKernel(self.launcher.spatialPlotMin, self.launcher.spatialPlotMax, self.launcher.spatialPlotSize, self.launcher.spatialPlotSize)

    def minimumSizeHint(self):
        return QSize(256, 256)
