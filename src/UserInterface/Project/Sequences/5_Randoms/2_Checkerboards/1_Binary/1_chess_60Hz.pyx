from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.RandomGrid(
					randomGridSize = (38, 38) ,
					spatialFilter = Spatial.Median() ,
					duration_s = 480,
					randomSeed = 35436546,
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Random chessboard',
		frameRateDivisor=1
		).setAgenda( agenda )
