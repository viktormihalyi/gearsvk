from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 3  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999*2,
                        ),
				Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999*1,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/2,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/3,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/4,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/5,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/6,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/10,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/15,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/30,
                        ),
				Stimulus.Blank( duration_s = 6  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Fullfield sine wave oscillations').setAgenda( agenda )
