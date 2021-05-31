from Project.Components import *
import random

def create(mediaWindow):
	multifig = Pif.Spot(
				radius = 30,
				)
	for i in range(0, 30) :
		multifig = multifig + (Pif.Spot(
								radius = 30,
								)
								<< Motion.Linear(
									startPosition = (random.uniform(-1000, 1000), random.uniform(-1000, 1000)),
									velocity = (random.uniform(-50, 50), random.uniform(-50, 50)),
									scaleVelocity = (random.uniform(-0.1, 0.1), random.uniform(-0.1, 0.1)),
									)
								)
	agenda = [
			Stimulus.Blank( duration_s = 0.2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 0.2  ),
			Stimulus.SinglePass(
					duration_s = 20,
					spass = Pass.Generic(
						pif = multifig,
						),
					),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence(
		'Varius shapes',
		frameRateDivisor=1
		).setAgenda( agenda )
