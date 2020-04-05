from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 4  ),
				Stimulus.Fullfield( duration_s = 0.0166 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 0.0333 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 0.0666 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 0.125 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 0.25 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 0.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 1 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 1.5 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 2 ),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.Fullfield( duration_s = 4 ),
				Stimulus.Blank( duration_s = 2.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Increasing time fullfields').setAgenda( agenda )
