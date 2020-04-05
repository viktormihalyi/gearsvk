from Project.Components import *


def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 2) :
		agenda += [
				Stimulus.Blank( 
						duration_s = 60,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0.4,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0.6,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0.3,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0.7,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0.2,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0.8,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Fullfield( 
						duration_s = 2,
						color = 0,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Blank( 
						duration_s = 2,
						color = 1,
						),
				Stimulus.Blank( 
						duration_s = 10,
						color = 0.5,
						),
				Stimulus.Blank( duration_s = 10  ),
				Stimulus.Fullfield( 
						duration_s = 2,
						),
				Stimulus.Blank( duration_s = 10  ),
				]
	agenda += [
			Stimulus.Blank( duration_s = 4.5  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Increasing constrast').setAgenda( agenda )
