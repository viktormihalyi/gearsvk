from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	agenda += [
			Stimulus.Blank( duration_s = 13  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'east',
					start = -0.1,
					end = 0.1,
					),
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'west',
					start = -0.1,
					end = 0.1,
					),
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'north',
					start = -0.1,
					end = 0.1,
					),
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'south',
					start = -0.1,
					end = 0.1,
					),
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'northwest',
					start = -0.1,
					end = 0.1,
					),			
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'southeast',
					start = -0.1,
					end = 0.1,
					),			
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'northeast',
					start = -0.1,
					end = 0.1,
					),
			Stimulus.Blank( duration_s = 26  ),
			Stimulus.FullfieldGradient(
					duration_s = 4,
					direction = 'southwest',
					start = -0.1,
					end = 0.1,
					),			
			Stimulus.Blank( duration_s = 26  ),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Various edges').setAgenda( agenda )
