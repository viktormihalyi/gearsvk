from Project.Components import *

def create(mediaWindow):
    return ImageSequence('Rotating snake').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Image    ( 
					duration_s = 20,
					imagePath='./Project/Media/rotsnake.jpg',
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

