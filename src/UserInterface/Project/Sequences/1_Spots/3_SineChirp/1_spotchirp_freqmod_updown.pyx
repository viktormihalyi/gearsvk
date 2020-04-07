from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 2  ),
				Stimulus.SpotOscillation    ( duration_s = 37.5, radius = 125, wavelength_s=4.2709376042709376042709376042709, endWavelength_s=0.0667334 ),
				Stimulus.SpotOscillation    ( duration_s = 37.5, radius = 125, wavelength_s=0.0667334, endWavelength_s=4.2709376042709376042709376042709),
				Stimulus.Blank( duration_s = 4.5  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Spots with linearly changing modulation frequencies').setAgenda( agenda )

