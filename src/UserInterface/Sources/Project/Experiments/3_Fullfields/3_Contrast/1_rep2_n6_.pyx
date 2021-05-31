from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 2) :
		agenda += [
				Stimulus.Blank( 
						duration_s = 40,
						intensity = 0.5,
						),
				]
		for j in range(0, 6) :
			agenda += [
				Stimulus.Fullfield( 
						duration_s = 2,
						intensity = 1,
						),
				Stimulus.Blank( 
						duration_s = 6.5,
						intensity = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						intensity = 0,
						),
				Stimulus.Blank( 
						duration_s = 6.5,
						intensity = 0.5,
						),
				]
	agenda += [
			Stimulus.Blank( duration_s = 4.5  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Contrast binary').setAgenda( agenda )
