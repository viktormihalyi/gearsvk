from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Video-to-texture test').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Video    ( 
					duration_s = 5,
					#videoPath = './Project/Media/slam.avi',
					videoPath = './Project/Media/small.mp4',
					#videoPath = './Project/OutputVideos/7_VirtualEnvironments/2_3DEnvironments/2_Raycast/2_volcanic.mpg',
					#videoPath = './Project/OutputVideos/7_VirtualEnvironments/4_MovingShapes/6_Complex/6_Complex.mpg',
                    toneMapping = Tone.Linear(
							dynamic = True
							),
					),
            Stimulus.Video    ( 
					duration_s = 5,
					#videoPath = './Project/Media/slam.avi',
					videoPath = './Project/Media/small.mp4',
					#videoPath = './Project/OutputVideos/7_VirtualEnvironments/2_3DEnvironments/2_Raycast/2_volcanic.mpg',
					#videoPath = './Project/OutputVideos/7_VirtualEnvironments/4_MovingShapes/6_Complex/6_Complex.mpg',
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

