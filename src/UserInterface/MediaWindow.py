import Gears as gears
from PyQt5.QtCore import (Qt, QTimer, QUrl, QSizeF, QEvent)
from PyQt5.QtMultimedia import (QMediaContent, QMediaPlayer, QVideoProbe)
from PyQt5.QtMultimediaWidgets import (QGraphicsVideoItem)
from PyQt5.QtWidgets import (QApplication, QWidget, QGraphicsScene, QGraphicsView, QVBoxLayout)
import datetime as datetime

class MediaWindow(QWidget) :
    def __init__(self, stimulusWindow):
        super(MediaWindow, self).__init__()
        self.setFocusPolicy(Qt.StrongFocus)
        self.setStyleSheet( '''
            QWidget{
                border-style: none;
            }
            '''
            )

        #self.setWindowFlags(Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint)

        self.showFullScreen()
        
        self.stimulusWindow = stimulusWindow
        stimulusWindow.showFullScreen()

        self.mediaPlayer= QMediaPlayer(None, QMediaPlayer.VideoSurface)

        self.videoProbe = QVideoProbe()
        self.videoProbe.videoFrameProbed.connect(self.onVideoFrame)
        self.videoProbe.setSource(self.mediaPlayer)

        self.graphicsVideoItem = QGraphicsVideoItem()

        self.graphicsScene = QGraphicsScene()
        self.graphicsView = QGraphicsView(self.graphicsScene)
        geom = stimulusWindow.geometry()
        self.graphicsView.setFixedSize(geom.width(), geom.height())
        self.graphicsScene.addItem(self.graphicsVideoItem)

        self.graphicsView.setViewport(self.stimulusWindow)
        self.graphicsView.setViewportUpdateMode(QGraphicsView.FullViewportUpdate)

        print(self.graphicsView.contentsRect())

        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.alignmentRect
        layout.addWidget(self.graphicsView)
  
        self.setLayout(layout)
        self.graphicsView.setFocusPolicy(Qt.NoFocus)
        self.graphicsView.viewport().installEventFilter(self)
        self.hide()
        #self.showFullScreen()

        self.ticker = None

    def eventFilter(self, object, event) :
        if event.type() == QEvent.MouseMove :
            self.mouseMoveEvent(event)
        return False

    def playVideo(self, videoPath, duration):
        self.showFullScreen()

        url = QUrl.fromLocalFile(videoPath)
        self.mediaContent= QMediaContent( url )

        self.mediaPlayer.setVideoOutput(self.graphicsVideoItem)
        self.mediaPlayer.setMedia(self.mediaContent)

        self.stimulusWindow.measuredFrameInterval_s = self.stimulusWindow.expectedFrameInterval_s
        self.stimulusWindow.gracePeriodForFpsMeasurement = 60
        print('mediaplayer: video start')
        print(self.mediaPlayer.mediaStatus() )
        self.timer.stop()
        self.mediaPlayer.play()
        self.framesToGo = duration
        print('frames to go: ' + str(self.framesToGo))

    def stopVideo(self):
        #self.stimulusWindow.showFullScreen()
        print('mediaplayer: video stop')
        self.mediaPlayer.stop()
        self.timer.start()

    def onVideoFrame(self, frame):
        gears.onDisplayHidden()
        self.framesToGo = self.framesToGo-1
        if self.framesToGo == 0 :
            self.stopVideo()

    def start(self, launcherWindow):
        print("Starting media window.")
        print(datetime.datetime.now().time())

        self.launcherWindow = launcherWindow
        sequence = gears.getSequence().getPythonObject()
        self.expectedFrameInterval_s = sequence.getFrameInterval_s()
        self.measuredFrameInterval_s = self.expectedFrameInterval_s
        self.gracePeriodForFpsMeasurement = 60

        if sequence.getUsesBusyWaitingThreadForSingals():
            self.ticker = gears.startTicker()
        else:
            self.ticker = None
        self.timer = QTimer(self)
        self.timer.setInterval(0)
        self.timer.timeout.connect(self.onTimer)

        self.stimulusWindow.mediaWindow = self
        self.timer.start()
        self.firstFrame = True
        self.setFocus()

        print("Showing media window.")
        print(datetime.datetime.now().time())

        self.showFullScreen()

    def stop(self):
        self.timer.stop()
        sequence = gears.getSequence().getPythonObject()
        if self.ticker:
            self.ticker.stop()
        self.launcherWindow.wake()
        QApplication.instance().processEvents()
        gears.reset()
        self.hide()
        self.lower()

    def onBufferSwap(self):
        if self.ticker:
            self.ticker.onBufferSwap()

    def onTimer(self):
        #print("Media window timer tick.")
        #print(datetime.datetime.now().time())
        self.stimulusWindow.updateNative()

    def keyReleaseEvent(self, event):
        stimulus = gears.getCurrentStimulus().getPythonObject()
        try:
            for cb in stimulus.onKeyUp :
                cb(event)
        except AttributeError :
            pass

    def keyPressEvent(self, event):
        sequence = gears.getSequence().getPythonObject()
        if event.text() == 'q' or event.key() == Qt.Key_Escape :
            gears.skip(100000000)
        elif event.text() == 'f':
            self.showFps = not self.showFps
        elif event.key() == Qt.Key_Right or event.text() == 's':
            gears.skip(1)
        elif event.key() == Qt.Key_Left or event.text() == 'b':
            gears.skip(-1)
        elif event.text() == 'p' or event.text() == 'a':
            gears.pause()
        elif event.text() == 'y':
            gears.instantlyClearSignal(sequence.onKeySpikeChannel)
            gears.instantlyRaiseSignal(sequence.onKeySpikeChannel)
        elif event.text() == 'h':
            gears.instantlyRaiseSignal(sequence.onKeySpikeChannel)
            gears.instantlyClearSignal(sequence.onKeySpikeChannel)
        else:
            if event.text() == ' ':
                gears.setText("_____toggle info", "Press SPACE to hide this text.")
                gears.showText()
            stimulus = gears.getCurrentStimulus().getPythonObject()
            try:
                for cb in stimulus.onKey :
                    cb(event)
            except AttributeError :
                pass


    def mouseMoveEvent(self, event):
        stimulus = gears.getCurrentStimulus().getPythonObject()
        try:
            for cb in stimulus.onMouse :
                cb(event)
        except AttributeError :
            pass

    def mousePressEvent(self, event):
        stimulus = gears.getCurrentStimulus().getPythonObject()
        try:
            for cb in stimulus.onMouseClick :
                cb(event)
        except AttributeError :
            pass

    def wheelEvent(self, event):
        stimulus = gears.getCurrentStimulus().getPythonObject()
        try:
            for cb in stimulus.onWheel :
                cb(event)
        except AttributeError :
            pass

