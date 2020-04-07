from Project.Components import *
from PyQt5.QtCore import (QTimer, QUrl, QSizeF)
from PyQt5.QtMultimedia import (QMediaContent, QMediaPlayer, QVideoProbe)
from PyQt5.QtMultimediaWidgets import (QGraphicsVideoItem)
from PyQt5.QtWidgets import (QGraphicsScene, QGraphicsView)

class VideoStimulus(Stimulus.Generic) :
    def boot(self,
            duration       =   1,
            duration_s     =   0,
            videoPath      =   None,
            mediaWindow    =   None,
            ):
        super().boot(
                name='Video',
                duration=duration,
                duration_s=duration_s
            )
        sequence = self.getSequence()
        self.mediaWindow = mediaWindow
        self.videoPath = videoPath

        stimulus.registerCallback(gears.BeginStimulus(), self.play)
        stimulus.registerCallback(gears.FinishStimulus(), self.stop)

    def play(self):
        print('stim: video start')
        self.mediaWindow.playVideo(self.videoPath, self.duration)

        #self.stimulusWindow.measuredFrameInterval_s = self.stimulusWindow.expectedFrameInterval_s
        #self.stimulusWindow.gracePeriodForFpsMeasurement = 60

        print(self.mediaWindow.mediaPlayer.mediaStatus() )

    def stop(self):
        self.mediaWindow.stopVideo()
        print('stim: video stop')


