from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank(
					color = 0.5 , duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Spot   ( duration_s = 2,	radius = 150 ),
				Stimulus.Blank	( duration_s = 6.5  ),
				Stimulus.Spot   ( duration_s = 2,	radius = 5000 ),
				Stimulus.Blank	( duration_s = 6.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Spot and full field').setAgenda( agenda )

