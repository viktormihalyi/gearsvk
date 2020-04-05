from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.SaturatedColorRandomGrid(
					duration_s = 60,
					randomSeed = 35436546,
					randomGridSize = (128, 128),
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Hi-res random chessboard',
		frameRateDivisor=1
		).setAgenda( agenda )
