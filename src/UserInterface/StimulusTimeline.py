import sys
import Gears as gears
import importlib.machinery
import os
import GearsUtils as utils
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer, QSize)
from PyQt5.QtWidgets import (QWidget, QToolTip, QPushButton, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout)
from PyQt5.QtGui import (QFont, QPalette, QFontMetrics, QOpenGLContext, QPainter )
from PyQt5.QtOpenGL import (QGLWidget, QGLFormat, QGLContext)
try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
except:
  print ('ERROR: PyOpenGL not installed properly.')


class StimulusTimeline(QGLWidget):
    width = 128
    height = 128
    luncher = None
    stimulusTimelineStartFrame = 0
    stimulusTimelineFrameCount = 64
    margin = 80
    fontMetrics = None

    def __init__(self, parent, launcher, winId):
        utils.initQGLWidget(self, super(), parent, winId)
        self.launcher = launcher
        self.fontMetrics = QFontMetrics(self.font())

    def initializeGL(self):
        err = glGetError()
        if(err):
            print("An OpenGL error occcured in PyQt. A known cause for this is a driver problem with Intel HD graphics chipsets. Try updating your driver, manually if necessary.")
            print("OpenGL error code: " + str(err))

    def resizeGL(self, w, h):
        self.width = w
        self.height = h
        glViewport(0, 0, w, h)
        self.fontMetrics = QFontMetrics(self.font())

    #def renderText(self, x, y, text, font=None):
    #    if font == None :
    #        self.painter.setFont(self.font())
    #    else :
    #        self.painter.setFont(font)
    #    self.painter.drawText(x, y, text)

    def paintGL(self):
        #self.painter = QPainter(self)
        #self.painter.beginNativePainting()
        gears.drawStimulusTimeline(self.margin, 0, self.width-self.margin, self.height)
        #self.painter.endNativePainting()
        emHeight = self.fontMetrics.height()
        emWidth = self.fontMetrics.width('M')
        self.renderText(0, self.height * (0.1), 'Intensity' )
        self.renderText(self.margin - emWidth, self.height * 0.0 + emHeight * 0.5, '1' )
        self.renderText(self.margin - 2*emWidth, self.height * 0.2 + emHeight * 0.25, '0.5' )
        self.renderText(self.margin - emWidth, self.height * 0.4 + emHeight * 0.0, '0' )

        stim = gears.getSelectedStimulus()
        expr = gears.getSequence()
        channels = expr.getChannels()
        nChannels = stim.getChannelCount()
        i = 0
        for channel in channels:
            if stim.usesChannel(channel.key()) :
                if channel.data().raiseFunc == 3 or channel.data().raiseFunc == 4 :
                    signal = 'RTS'
                elif channel.data().raiseFunc == 8 or channel.data().raiseFunc == 9 :
                    signal = 'BREAK'
                elif channel.data().raiseFunc == 5 or channel.data().raiseFunc == 6 :
                    signal = 'DTR'
                else:
                    signal = '???'
                channelZoneHeight = self.height / 2 / nChannels
                lpos =  self.height * 0.5 + channelZoneHeight * i
                self.renderText(0, lpos +  channelZoneHeight * 0.5, channel.key() )
                self.renderText(0, lpos +  channelZoneHeight * 0.5 + emHeight * 0.5, channel.data().portName + ':' + signal,
                               QFont('Candara', 6) )

                self.renderText(self.margin - emWidth, lpos +  channelZoneHeight * 0.5 + emHeight * 0.25, '1' )
                self.renderText(self.margin - emWidth, lpos +  channelZoneHeight * 0.75 + emHeight * 0.25, '0' )

                i += 1

        #stim = gears.getSelectedStimulus()
        #self.renderText(stim.data().getStartingFrame() * self.width / expr.getDuration(), 50, str(stim.data().getDuration()) )
        self.painter = None

    def minimumSizeHint(self):
        return QSize(1024, 300)

    def mousePressEvent(self, e):
        self.prevMouseX = e.x()

    def mouseMoveEvent(self, e):
        self.launcher.stimPan(e.x() - self.prevMouseX)
        self.prevMouseX = e.x()

    def wheelEvent(self, e):
        self.launcher.stimZoom(e)

