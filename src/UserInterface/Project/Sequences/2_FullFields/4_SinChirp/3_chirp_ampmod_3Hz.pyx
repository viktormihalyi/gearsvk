from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			Stimulus.Blank( 
						duration_s = 45, 
						color = 0.5,
						),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( 
						duration_s = 2, 
						color = 0.5,
						),
				Stimulus.FullfieldOscillation    ( 
						duration_s = 40, 
						wavelength_s=0.333, 
						amplitude=0,   
						endAmplitude=0.5,
						),
				Stimulus.Blank( 
						duration_s = 4.5, 
						color = 0.5,
						),
				Stimulus.FullfieldOscillation    (
						duration_s = 40,
						wavelength_s=0.333,
						amplitude=0.5,
						endAmplitude=0,
						),
				Stimulus.Blank( 
						duration_s = 4.5, 
						color = 0.5,
						),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Fullfield with linearly changing modulation amplitude').setAgenda( agenda )

