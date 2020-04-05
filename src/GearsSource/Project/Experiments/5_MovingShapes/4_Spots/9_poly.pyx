from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	agenda += [
			Stimulus.SinglePass    (
					duration_s = 10,
					spass = Pass.Generic(
							pif = Pif.Spot(
									radius = 300,
									innerRadius = 50
									),
							rasterizingMode = 'triangles',
							polygonMask = [{'x':0, 'y':0}, {'x':0, 'y':100}, {'x':100, 'y':0}],
							),
					)
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return MovingShapeSequence('Spots with increasing modulation frequencies with shaking').setAgenda( agenda )

