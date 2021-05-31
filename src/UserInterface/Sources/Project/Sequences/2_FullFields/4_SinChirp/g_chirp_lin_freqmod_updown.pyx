from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.FullfieldOscillation    ( 
					duration_s = 6,
					phase = 3.1415/2,
					wavelength_s=1,
					endWavelength_s=1/6,
					linearFrequencyChange = True,
					),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Fullfields with linearly changing modulation frequencies').setAgenda( agenda )

