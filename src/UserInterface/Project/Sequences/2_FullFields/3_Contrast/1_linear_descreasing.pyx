from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 2) :
		agenda += [
				Stimulus.Blank( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 16,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 8,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 4,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 2,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 1,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 0.5,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 0.25,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 6.5  ),
				Stimulus.FullfieldLinearModulation( duration_s = 2  ),
				Stimulus.FullfieldLinearModulation(
						duration_s = 0.125,
						intensitySlope = 'down',
						),
				Stimulus.Blank( duration_s = 4.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Linear downlight fades').setAgenda( agenda )
