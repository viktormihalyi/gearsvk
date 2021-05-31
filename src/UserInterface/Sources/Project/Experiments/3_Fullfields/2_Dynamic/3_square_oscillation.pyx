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
						exponent = 0.01,
                        ),
				Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999*1,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/2,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/3,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/4,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/5,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/6,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/7,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/8,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/9,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/10,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/11,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/12,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/15,
						exponent = 0.01,
                        ),
                Stimulus.Blank( duration_s = 6  ),
				Stimulus.FullfieldOscillation(
                        duration_s = 6,
                        wavelength_s=1/0.999/30,
						exponent = 0.01,
                        ),
				Stimulus.Blank( duration_s = 6  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Fullfield square wave oscillations').setAgenda( agenda )
