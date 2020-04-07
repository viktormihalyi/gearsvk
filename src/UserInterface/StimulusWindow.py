import sys
import Gears as gears
import importlib.machinery
import os
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer, QThread, pyqtSignal)
from PyQt5.QtWidgets import (QWidget, QToolTip, QPushButton, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout, QSpacerItem, QSizePolicy)
from PyQt5.QtGui import (QFont, QPalette, QColor, QPainter)
from PyQt5.QtMultimedia import QSound
from PyQt5.QtOpenGL import (QGLWidget, QGLFormat)
try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
except:
  print ('ERROR: PyOpenGL not installed properly.')
import time as time
import datetime as datetime

#class Ticker(QThread) :
#    live = True
#    def __init__(self, glWidget, tickInterval):
#        super().__init__()
#        self.glWidget = glWidget
#        self.tickInterval = tickInterval
#    def run(self) :
#        prevTick = time.time()
#        while(self.live):
#            now = time.time()
#            if now > prevTick:
#                 prevTick = prevTick + self.tickInterval
#                 self.glWidget.updateSignal.emit()
#                 gears.tick()

class StimulusWindow(QGLWidget):
    timer = None
    launcherWindow = None
    updateSignal = pyqtSignal()
    prevFrameTime = None
    expectedFrameInterval_s = 0.999
    measuredFrameInterval_s = 0.999
    showFps = False

    def __init__(self, parent = None):
        self.stimulusWindow = self
        format = QGLFormat()
        format.setSwapInterval(1)
        super().__init__(format, parent)
        #super().__init__(parent)
        self.setFocusPolicy(Qt.NoFocus)
        self.updateSignal.connect(self.updateGL)
        self.setStyleSheet("""
            QWidget{
                background-color: black;
                border-color: red;
                border-style: solid;
                border-width: 1px;
                border-radius: 0px;
                font: bold 14px;
                color: red;
                padding: 0px;
                font-family: "Candara";
                font-size: 14px;
            }
            """ )
        self.setCursor(Qt.BlankCursor)
        self.fpsWarningSound = QSound("./Project/Media/airhorn.wav")
        self.finishedSound = QSound("./Project/Media/FANFARE.WAV")

        self.handle = None
        self.dllref = None
            
    def initializeGL(self):
        try:
            print("Initializing stimulus window.")
            print(datetime.datetime.now().time())

            self.specs = gears.init ()

            print("Stimulus window initialized.")
            print(datetime.datetime.now().time())

        except RuntimeError as e:
            box = QMessageBox(self)
            horizontalSpacer = QSpacerItem(1000, 0, QSizePolicy.Minimum, QSizePolicy.Expanding)
            box.setText("Runtime error: {0}".format(e)  )
            box.setWindowTitle('Error initializing OpenGL!\n')
            box.setWindowFlags(Qt.Dialog);
            box.setStandardButtons(QMessageBox.Abort)
            box.setDefaultButton(QMessageBox.Abort)
            layout = box.layout()
            layout.addItem(horizontalSpacer, layout.rowCount(), 0, 1, layout.columnCount())
            box.exec()
            os._exit(-1)
 
    def onTimer(self):
        print(time.time())
        self.update()

    def keyReleaseEvent(self, event):
        pass

    def updateNative(self):
        #print("Drawing frame.")
        #print(datetime.datetime.now().time())

        self.makeCurrent()
        if not gears.onDisplay(0) :
            self.mediaWindow.stop()
            self.finishedSound.play()

        self.swapBuffers()
        glFinish()

        self.mediaWindow.onBufferSwap()

    #def paintGL(self):
    #    if self.frozen :
    #        return
    #    self.mediaWindow.onBufferSwap()
    #    if not gears.onDisplay(0) : #/*self.defaultFramebufferObject()*/
    #        self.mediaWindow.stop()
    #        self.finishedSound.play()
        #else :
        #    now = time.time()
        #    if self.gracePeriodForFpsMeasurement < 0 :
        #        if self.firstFrame :
        #            self.prevFrameTime = time.time()
        #            self.measuredFrameInterval_s = gears.getSequence().getFrameInterval_s()
        #            self.firstFrame = False
        #        else :
        #            dt = now - self.prevFrameTime
        #            self.prevFrameTime = now
        #            self.measuredFrameInterval_s = self.measuredFrameInterval_s * 0.95 + 0.05 * dt
        #            if self.showFps :
        #                if self.measuredFrameInterval_s > gears.getSequence().getFrameInterval_s() * 1.01 :
        #                    self.renderText(0, 500, 'FPS warning! Current FPS: '+  ("%.2f" % (1/self.measuredFrameInterval_s)), QFont('Candara', pointSize=40) )
        #                    if self.fpsWarningSound.isFinished() :
        #                        self.fpsWarningSound.play()
        #                else :
        #                    self.renderText(0, 500, 'FPS OK! Current FPS: '+  ("%.2f" % (1/self.measuredFrameInterval_s)), QFont('Candara', pointSize=40) )
        #                    self.renderText(0, 550, 'Press F again to remove FPS.', QFont('Candara', pointSize=20) )
        #    self.gracePeriodForFpsMeasurement = self.gracePeriodForFpsMeasurement-1
        
    #def mouseMoveEvent(self, event):
    #    print('moveit')
    #    stimulus = gears.getCurrentStimulus().getPythonObject()
    #    try:
    #        stimulus.onMouse(event)
    #    except AttributeError :
    #        pass
    #
    #def mousePressEvent(self, event):
    #    print('clickit')
    #    stimulus = gears.getCurrentStimulus().getPythonObject()
    #    try:
    #        stimulus.onMouseClick(event)
    #    except AttributeError :
    #        pass