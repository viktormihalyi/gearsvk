from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.RandomGrid(
					duration_s = 480,
					randomSeed = 35436546,
					spatialFilter = Spatial.BigDog(),
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return RandomGridSequence(
		'Big DOG chessboard',
		frameRateDivisor=1
		).setAgenda( agenda )

