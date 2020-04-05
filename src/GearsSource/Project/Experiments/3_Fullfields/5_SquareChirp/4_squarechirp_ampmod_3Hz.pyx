from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			Stimulus.Blank( 
						duration_s = 45, 
						intensity = 0.5,
						),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( 
						duration_s = 2, 
						intensity = 0.5,
						),
				Stimulus.FullfieldOscillation    ( 
						duration_s = 40, 
						wavelength_s=0.333, 
						amplitude=0,   
						endAmplitude=0.5,
						exponent = 0.01,
						),
				Stimulus.Blank( 
						duration_s = 4.5, 
						intensity = 0.5,
						),
				Stimulus.FullfieldOscillation    (
						duration_s = 40,
						wavelength_s=0.333,
						amplitude=0.5,
						endAmplitude=0,
						exponent = 0.01,
						),
				Stimulus.Blank( 
						duration_s = 4.5, 
						intensity = 0.5,
						),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return FullfieldSequence('Spots with linearly changing modulation frequencies').setAgenda( agenda )

