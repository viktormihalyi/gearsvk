from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.SingleShape(
					duration_s = 480,
					shape = Pif.RandomGrid(
						) << Warp.Polarize(
							radius = 1000,
							innerRadius = 300,
							endAngle = 2,
							),
					prng =  Prng.XorShift128(
						randomSeed = 35436546,
						randomGridSize = (64, 8),
						),
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return MaskedRandomGridSequence(
		'Random chessboard in polar coordinates',
		frameRateDivisor=1
		).setAgenda( agenda )
