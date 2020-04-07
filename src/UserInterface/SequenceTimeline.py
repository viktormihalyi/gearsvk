import sys
import Gears as gears
import importlib.machinery
import os
import GearsUtils as utils
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer, QSize, QThread)
from PyQt5.QtWidgets import (QWidget, QToolTip, QPushButton, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout)
from PyQt5.QtGui import (QFont, QPalette, QFontMetrics, QOpenGLContext, QPainter )
from PyQt5.QtOpenGL import (QGLWidget, QGLFormat, QGLContext)
try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
  from OpenGL.GLX import *
except:
  print ('ERROR: PyOpenGL not installed properly.')

class SequenceTimeline(QGLWidget):
    width = 128
    height = 128
    launcher = None
    fontMetrics = None
    sequenceTimelineStartFrame = 0
    sequenceTimelineFrameCount = 1024
    prevMouseX = 0
    margin = 80

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
        #print("resize")
        #print('w: ' + str(w) + ', h: ' + str(h))
        #print('+valid: ', self.isValid())
       # print(glGetError())
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
        gears.drawSequenceTimeline(self.margin, 0, self.width-self.margin, self.height)
        #self.painter.endNativePainting()
        expr = gears.getSequence()
        emHeight = self.fontMetrics.height()
        emWidth = self.fontMetrics.width('M')
        self.renderText(0, self.height * 0.75*0.5, 'Intensity' )
        self.renderText(self.margin - emWidth, self.height * 0.5*0.5 + emHeight * 0.25, '1' )
        self.renderText(self.margin - 2*emWidth, self.height * 0.5*0.7 + emHeight * 0.25, '0.5' )
        self.renderText(self.margin - emWidth, self.height * 0.5*0.9 + emHeight * 0.25, '0' )
        
        #self.renderText(0, self.height * 0.465 + emHeight, str(self.sequenceTimelineStartFrame) + ' fr --' )
        #maxText = '-- ' + str(self.sequenceTimelineStartFrame + self.sequenceTimelineZoom ) + ' fr'
        #maxiWidth = self.fontMetrics.width(maxText)
        #self.renderText(self.width - maxiWidth, self.height * 0.465 + emHeight,  maxText)
        #self.renderText(0, self.height * 0.465 + 2.5*emHeight, ( '%.2f' % (self.sequenceTimelineStartFrame * expr.getFrameInterval_s())) + ' s --' )
        #smaxText = '-- ' + ('%.2f' % ((self.sequenceTimelineStartFrame + self.sequenceTimelineZoom)  * expr.getFrameInterval_s() ) )+ ' s'
        #smaxiWidth = self.fontMetrics.width(smaxText)
        #self.renderText(self.width - smaxiWidth, self.height * 0.465 + 2.5*emHeight,  smaxText)
        
        maxText = str(self.sequenceTimelineStartFrame) + ' - ' + str(self.sequenceTimelineStartFrame + self.sequenceTimelineZoom ) + ' frames in overview'
        maxiWidth = self.fontMetrics.width(maxText)
        self.renderText(self.width - maxiWidth, emHeight,  maxText)
        smaxText = ( '%.2f' % (self.sequenceTimelineStartFrame * expr.getFrameInterval_s())) + ' s - ' + ('%.2f' % ((self.sequenceTimelineStartFrame + self.sequenceTimelineZoom)  * expr.getFrameInterval_s() ) )+ ' s time in overview'
        smaxiWidth = self.fontMetrics.width(smaxText)
        self.renderText(self.width - smaxiWidth, emHeight * 2,  smaxText)
        
        self.renderText(0.0, emHeight * 1, str(expr.getDuration()) + ' frames total')
        self.renderText(0.0, emHeight * 2, '%.2f' % (expr.getDuration()*expr.getFrameInterval_s()) + ' s time total')
        
        stimuli = expr.getStimuli()
        for stim in stimuli:
            nText = stim.data().name
            nWidth = self.fontMetrics.width(nText)
            if nWidth < (self.width-self.margin) / self.sequenceTimelineZoom * stim.data().getDuration() :
                self.renderText(self.margin+(stim.data().getStartingFrame() - self.sequenceTimelineStartFrame + stim.data().getDuration()/2 ) * (self.width-self.margin) / self.sequenceTimelineZoom - nWidth/2, 
                                emHeight * 3, nText )
            #frText = str(stim.data().getDuration()) + ' fr'
            #frWidth = self.fontMetrics.width(frText)
            #if frWidth < self.width / self.sequenceTimelineZoom * stim.data().getDuration() :
            #    self.renderText(self.margin+(stim.data().getStartingFrame() - self.sequenceTimelineStartFrame + stim.data().getDuration()/2 ) * (self.width-self.margin) / self.sequenceTimelineZoom - frWidth/2, 
            #                    emHeight * 4, frText )
            sText = '%.2f' % stim.data().getDuration_s() + ' s'
            sWidth = self.fontMetrics.width(sText)
            if sWidth < self.width / self.sequenceTimelineZoom * stim.data().getDuration() :
                self.renderText(self.margin+(stim.data().getStartingFrame() - self.sequenceTimelineStartFrame + stim.data().getDuration()/2 ) * (self.width-self.margin) / self.sequenceTimelineZoom - sWidth/2, 
                                emHeight * 4, sText )
        channels = expr.getChannels()
        nChannels = expr.getChannelCount()
        i = 0
        for channel in channels:
            if channel.data().raiseFunc == 3 or channel.data().raiseFunc == 4 :
                signal = 'RTS'
            elif channel.data().raiseFunc == 8 or channel.data().raiseFunc == 9 :
                signal = 'BREAK'
            elif channel.data().raiseFunc == 5 or channel.data().raiseFunc == 6 :
                signal = 'DTR'
            else:
                signal = '???'
            #cName = channel.key() + ' ------ signal ' + signal + ' on ' + channel.data().portName
            channelZoneHeight = self.height / 2 / nChannels
            lpos =  self.height / 2 + channelZoneHeight * i
            self.renderText(0, lpos +  channelZoneHeight * 0.5, channel.key() )
            self.renderText(10, lpos +  channelZoneHeight * 0.5 + emHeight * 0.75, channel.data().portName, QFont('Candara', 10) )
            self.renderText(10, lpos +  channelZoneHeight * 0.5 + emHeight * 1.5, signal, QFont('Candara', 10) )
        
            self.renderText(self.margin - emWidth, lpos +  channelZoneHeight * 0.5 + emHeight * 0.25, '1' )
            self.renderText(self.margin - emWidth, lpos +  channelZoneHeight * 0.75 + emHeight * 0.25, '0' )
        
            i += 1
        self.painter = None

    def mousePressEvent(self, e):
        if e.button() == Qt.LeftButton :
            expr = gears.getSequence()
            if e.pos().x() - self.margin < 0:
                gears.toggleChannelsOrPreview()
            else:
                gears.pickStimulus( ( (e.pos().x() - self.margin) / (self.width - self.margin) * self.sequenceTimelineZoom + self.sequenceTimelineStartFrame) / expr.getDuration(), e.pos().y() / self.height)
                self.launcher.stimulusChanged()
        self.prevMouseX = e.x()

    def mouseMoveEvent(self, e):
        self.launcher.pan(e.x() - self.prevMouseX)
        self.prevMouseX = e.x()

    # def minimumSizeHint(self):
    # Fullsize not working on linux because of this function
    #     return QSize(1024, 750)

    def wheelEvent(self, e):
        self.launcher.zoom(e)