from Project.Components import *

def create(mediaWindow):
	agenda = [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 1  ),
			Stimulus.Image( 
					duration_s = 40,
					imagePath='./Project/Media/ising.png',
					velocity = (200, 200),
					temporalFilter = Temporal.ExponentialLti(),
					),
            Stimulus.Blank( duration_s = 1  ),
			EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
			]
	
	return DefaultSequence('Ising').setAgenda( agenda )
