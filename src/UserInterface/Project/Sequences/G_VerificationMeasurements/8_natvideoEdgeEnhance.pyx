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
					spatialFilter = Spatial.DogFftFreqSpec(sigma1=5, weight1=220, sigma2=10, weight2=-220, offset=1),
					videoContrast = 0.2,
					videoBrightness = -0.8,
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

