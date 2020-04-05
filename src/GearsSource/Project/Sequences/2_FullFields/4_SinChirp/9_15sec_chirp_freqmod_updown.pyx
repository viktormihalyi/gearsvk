from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	agenda += [
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.FullfieldOscillation    (
					duration_s = 15,
					wavelength_s=1, 
					amplitude=0,   
					endAmplitude=0.5,
					),
			Stimulus.Blank( duration_s = 0.5  ),
			Stimulus.FullfieldOscillation    (
					duration_s = 15,
					wavelength_s=1, 
					amplitude=0.5,   
					endAmplitude=0,
					exponent = 0.001,
					),
			Stimulus.Blank( duration_s = 4.5  ),
			]
	agenda += [
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.FullfieldOscillation    ( duration_s = 15,    wavelength_s=4, endWavelength_s=1/15),
			Stimulus.Blank( duration_s = 0.5  ),
			Stimulus.FullfieldOscillation    (
					exponent = 0.001 , duration_s = 15, wavelength_s=1/15, endWavelength_s=4),
			Stimulus.Blank( duration_s = 4.5  ),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Fullfields with linearly changing modulation frequencies').setAgenda( agenda )

