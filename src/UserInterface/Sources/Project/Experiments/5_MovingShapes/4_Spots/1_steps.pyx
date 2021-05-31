from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	agenda += [
			Stimulus.Blank( duration_s = 2  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s= 120/59.94, motion=Motion.Shake(boundingRadius=100) ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=60/59.94, motion=Motion.Shake(boundingRadius=100) ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=20/59.94, motion=Motion.Shake(boundingRadius=100) ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=10/59.94, motion=Motion.Shake(boundingRadius=100) ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=6/59.94, motion=Motion.Shake(boundingRadius=100) ),
			Stimulus.Blank( duration_s = 6.5  ),
			Stimulus.SpotOscillation    ( duration_s = 6, radius = 125, wavelength_s=4/59.94, motion=Motion.Shake(boundingRadius=100) ),
			Stimulus.Blank( duration_s = 4.5  ),
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return MovingShapeSequence('Spots with increasing modulation frequencies with shaking').setAgenda( agenda )

