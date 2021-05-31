from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.GreyscaleRandomGrid(
					randomGridSize = (1, 1) ,
					duration_s = 480,
					randomSeed = 35436546,
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Greyscale random fullfield',
		frameRateDivisor=1
		).setAgenda( agenda )
