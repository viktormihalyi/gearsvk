from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	agenda += [
			Stimulus.Blank( duration_s = 2 , color=0.5,
				 temporalFilter = Temporal.Exponential(),
				),
			Stimulus.Spot( radius = 525, innerRadius = 200, background=0.5, color = 0.75, motion=Motion.Crossing(velocity=1000, direction='east'),
				 temporalFilter = Temporal.Exponential(),
				),
			Stimulus.Blank( duration_s = 4.5, color=0.5, 
				temporalFilter = Temporal.Exponential(),
				),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return MovingShapeSequence('Spots with increasing modulation frequencies with shaking').setAgenda( agenda )

