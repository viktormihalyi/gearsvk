from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Video with blur').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Video    ( 
					duration_s = 20,
					#videoPath = './Project/Media/natmouse.mp4',
					videoPath = './Project/Media/Birds_7.mp4',
					greyscale = True,
					spatialFilter = Spatial.DogFftSpatSpec(sigma1=100, weight1=8, weight2=0),
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

