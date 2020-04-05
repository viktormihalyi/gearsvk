from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 10  ),
				Stimulus.Fullfield( duration_s = 30  ),
				Stimulus.Blank( duration_s = 10  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Melanopsin').setAgenda( agenda )
