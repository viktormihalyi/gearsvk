from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.SinglePass(
					duration_s = 10,
					spass = Pass.Generic(
						pif =
								(Pif.Spot(
									radius = 300,
									innerRadius = 50
									)
									<< Modulation.Cosine(
										wavelength_s = 1,
									)) ** (Pif.Solid(color='red'), Pif.Solid(color='white'))
								*	
								(Pif.Rect(
										size_um = (500, 300),
									)
									<< Motion.Linear(
										#startPosition = (-1000, -1000),
										#velocity = (200, 200),
										startAngle = 1.5,
										angularVelocity = -0.1,
										scaleVelocity = (-0.1, 0.1),
									)) ** (Pif.SineWheel(color1='green'), Pif.Solid(color='white'))
								*	
								(Pif.Image(
									imagePath = './Project/Media/flying-big-bird.png',
									)
									<< Warp.Clamp()
									<< Motion.Linear(
										startPosition = (2000, 0),
										velocity = (-200, 10),
									))
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
