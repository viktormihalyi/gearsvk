from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 2  ),
				Stimulus.SpotOscillation    ( duration_s = 20, radius = 125, wavelength_s=0.1665, amplitude=0,   endAmplitude=0.5),
				Stimulus.SpotOscillation    ( duration_s = 20, radius = 125, wavelength_s=0.1665, amplitude=0.5, endAmplitude=0 ),
				Stimulus.Blank( duration_s = 4.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return SpotSequence('Spots with linearly changing modulation frequencies').setAgenda( agenda )

