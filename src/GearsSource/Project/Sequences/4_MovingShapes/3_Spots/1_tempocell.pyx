from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	agenda += [
			Stimulus.Blank( duration_s = 2 , color=0.5,
				 temporalFilter = Temporal.Cell(),
				),
			Stimulus.Spot( radius = 525, innerRadius = 200, background=0.5, color = 0.75, motion=Motion.Crossing(velocity=1000, direction='east'),
				 temporalFilter = Temporal.Cell(),
				),
			Stimulus.Blank( duration_s = 4.5, color=0.5, 
				temporalFilter = Temporal.Cell(),
				),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Moving spot with bimodal filter').setAgenda( agenda )

