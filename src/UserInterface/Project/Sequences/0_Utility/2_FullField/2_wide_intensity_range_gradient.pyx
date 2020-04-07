from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 1  ),
			StartMeasurement(),
			Stimulus.Blank( 
					duration_s = 1,
					color = 0.5,
					),
			Stimulus.FullfieldGradient( 
					duration_s = 1,
					),
			Stimulus.FullfieldGradient( 
					duration_s = 10,
					direction = 0,
					color1 = -1,
					color2 = 2,
					),
			EndMeasurement(),
			Stimulus.Blank( 
					duration_s = 1,
					color = 0.0,
					),
			]
	return DefaultSequence('Test calib').setAgenda( agenda )
