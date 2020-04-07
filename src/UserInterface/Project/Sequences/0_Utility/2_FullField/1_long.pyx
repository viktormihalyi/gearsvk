from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	agenda += [
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.Fullfield( duration_s = 200000  ),
			Stimulus.Blank( duration_s = 2  ),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Indefinite white').setAgenda( agenda )
