from Project.Components import *

def create(mediaWindow):
	agenda = [
			#Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			#Stimulus.Blank( duration_s = 2  ),
			Stimulus.GreyscaleRandomGrid(
					duration_s = 10,
					randomSeed = 35436546,
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            #Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Greyscale random chessboard',
		frameRateDivisor=1
		).setAgenda( agenda )
