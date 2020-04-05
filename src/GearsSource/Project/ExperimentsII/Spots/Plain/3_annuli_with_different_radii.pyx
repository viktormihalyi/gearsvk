from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 2  ),
				Stimulus.Spot    ( duration_s = 2, radius = 62.5, innerRadius = 0 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 125, innerRadius = 62.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 187.5, innerRadius = 125 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 250, innerRadius = 187.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 312.5, innerRadius = 250 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 375, innerRadius = 312.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 437.5, innerRadius = 375 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 500, innerRadius = 437.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 562.5, innerRadius = 500 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 5000, innerRadius = 562.5 ),
				Stimulus.Blank( duration_s = 4.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return SpotExperiment('Spots with increasing radii').setAgenda( agenda )

