from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	for i in range(0, 8) :
		agenda += [
				Stimulus.Blank( duration_s = 0.025  ),
				Stimulus.Fullfield( duration_s = 2  ),
				Stimulus.Blank( duration_s = 0.025  ),
				]
	agenda += [
            Stimulus.Blank( duration_s = 2  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Fullfield whites with short blacks').setAgenda( agenda )
