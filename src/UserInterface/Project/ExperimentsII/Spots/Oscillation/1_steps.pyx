from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	agenda += [
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=2 ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=1 ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=0.3333333333 ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=0.125 ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=0.1 ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=0.0666666666666 ),
			Stimulus.Blank( duration_s = 4.5  ),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return SpotExperiment('Spots with increasing modulation frequencies').setAgenda( agenda )

