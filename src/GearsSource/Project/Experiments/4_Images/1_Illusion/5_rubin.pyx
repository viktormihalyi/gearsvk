from Project.Components import *

def create(mediaWindow):
    return ImageSequence('Rubin vase').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SinglePass    (
            		spass = Pass.Generic(
							pif = Pif.Image(
									imagePath='./Project/Media/rubin.jpg',
									)
									** (Pif.SineGrating() * Pif.Solid(color=0.5) + Pif.Solid(color=0.25), Pif.Solid(color=1) ),
							), 
					duration_s = 20,
					),
            Stimulus.SinglePass    (
            		spass = Pass.Generic(
							pif = Pif.Image(
									imagePath='./Project/Media/rubin.jpg',
									)
									** (Pif.Solid(color=0), Pif.SineGrating(direction='north') * Pif.Solid(color=0.5) + Pif.Solid(color=0.25) ),
							), 
					duration_s = 20,
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

