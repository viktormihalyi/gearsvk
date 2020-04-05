from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.GreyscaleRandomGrid(
					#randomGridSize = (10, 10) ,
					duration_s = 10,
					randomSeed = 35436546,
					spatialFilter = Spatial.BigDog()
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Edge detected random grayscale chessboard',
		frameRateDivisor=1
		).setAgenda( agenda )
