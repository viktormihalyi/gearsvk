from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Video-to-texture test').setAgenda( [
            #Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Video    ( 
					duration_s = 8,
					#videoPath = './Project/Media/natmouse.mp4',
					videoPath = './Project/Media/Birds_7.mp4',
					temporalFilter = Temporal.Cell(),
					),
            EndMeasurement()             ,
            #Stimulus.Blank( duration_s = 1  )
        ])

