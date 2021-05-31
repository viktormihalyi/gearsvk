from Project.Components import *

def create(mediaWindow):
    return VideoSequence('Sample video').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement(),
			Stimulus.Blank( duration_s = 1  ),
            VideoStimulus    ( 
                    duration_s = 1,
                    #videoPath = './Project/Media/small.mp4',
					videoPath = './Project/Media/slam.avi',
                    mediaWindow = mediaWindow,
					),
			Stimulus.Blank( duration_s = 1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  )
        ])


