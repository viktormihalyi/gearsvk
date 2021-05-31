from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.RandomGrid(
					randomGridSize = (1, 1) ,
					duration_s = 480,
					randomSeed = 35436546,
					color1 = (0.75, 0.75, 0.75),
					color2 = (0.25, 0.25, 0.25),
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Random fullfield',
		frameRateDivisor=2
		).setAgenda( agenda )
