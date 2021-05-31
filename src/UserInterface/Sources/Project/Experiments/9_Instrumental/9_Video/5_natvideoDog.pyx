from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Video-to-texture test').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Video    ( 
					duration_s = 20,
					#videoPath = './Project/Media/natmouse.mp4',
					videoPath = './Project/Media/Birds_7.mp4',
					spatialFilter = Spatial.BigDog(),
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

