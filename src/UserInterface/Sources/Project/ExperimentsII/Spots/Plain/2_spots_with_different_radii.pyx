from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Spot    ( duration_s = 2, radius = 62.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 125 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 187.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 250 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 312.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Spot    ( duration_s = 2, radius = 5000 ),
				Stimulus.Blank( duration_s = 6.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return SpotExperiment('Spots with increasing radii').setAgenda( agenda )

