from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 1  ),
			StartMeasurement(),
			Stimulus.FullfieldGradient( 
					duration_s = 100,
					direction = 0,
					color1 = -1,
					color2 = 2,
#					toneMapping = Tone.Linear( 
#							dynamic = True,
#							toneRangeMin = Interactive.MouseWheel(
#									label = 'intensity mapped to zero',
#									initialValue = 0,
#									minimum = -10,
#									maximum = 10,
#									key = 'M',
#									),
#							toneRangeMax = Interactive.MouseWheel(
#									label = 'intensity mapped to one',
#									initialValue = 1,
#									minimum = -10,
#									maximum = 10,
#									key = 'X',
#									),
#						)
#					toneMapping = Tone.Erf( 
#						dynamic = True,
#						toneRangeMean = Interactive.MouseWheel(
#								label = 'intensity mean',
#								initialValue = 0.5,
#								minimum = -10,
#								maximum = 10,
#								key = 'M',
#								),
#						toneRangeVar = Interactive.MouseWheel(
#								label = 'intensity variance',
#								initialValue = 0.3,
#								minimum = 0,
#								maximum = 4,
#								key = 'X',
#								),
#						)
					toneMapping = Tone.HistogramEqualized( 
						dynamic = True,
						)


					),
			EndMeasurement(),
			Stimulus.Blank( 
					duration_s = 1,
					color = 0.0,
					),
			]
	return DefaultSequence('Test calib').setAgenda( agenda )
