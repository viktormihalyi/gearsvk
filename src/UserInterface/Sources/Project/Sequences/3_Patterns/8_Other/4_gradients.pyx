from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 4  ),
				Stimulus.FullfieldGradient(
						duration_s = 2,
						direction = 'east',
						),
				Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldGradient(
						duration_s = 2,
						direction = 'east',
						start = 0,
						),
				Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldGradient(
						duration_s = 2,
						direction = 'east',
						start = -0.1,
						end = -0.1,
						),
				Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldGradient(
						duration_s = 2,
						direction = 'northwest',
						),
				Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldGradient(
						duration_s = 2,
						direction = 'northwest',
						color1='yellow',
						color2='blue',
						start = -300,
						end = +300,
						),
				Stimulus.Blank( duration_s = 4.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Various fullfield gradients').setAgenda( agenda )
