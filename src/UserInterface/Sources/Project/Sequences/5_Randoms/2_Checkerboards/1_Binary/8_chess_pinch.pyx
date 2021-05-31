from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.SingleShape(
					duration_s = 480,
					shape = Pif.RandomGrid(
						) << Warp.PinchWhirl(
							radius = 700,
							),
					prng =  Prng.XorShift128(
						randomSeed = 35436546,
						),
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return MaskedDefaultSequence(
		'Random chessboard in polar coordinates',
		frameRateDivisor=1
		).setAgenda( agenda )
