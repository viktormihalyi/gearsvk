from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement(),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Blank( duration_s = 3  ),
				Stimulus.SinglePass(
                        duration_s = 30,
						spass = Pass.Generic(
							pif = Pif.Solid() 
								<< (Modulation.Cosine( wavelength_s = 10/5)
								*  Modulation.Cosine( wavelength_s = 10/7)
								*  Modulation.Cosine( wavelength_s = 10/11)
								*  Modulation.Cosine( wavelength_s = 10/13)
								*  Modulation.Cosine( wavelength_s = 1.5458)
								*  Modulation.Cosine( wavelength_s = 3.0045)
								*  Modulation.Cosine( wavelength_s = 2.4785)
								*  Modulation.Cosine( wavelength_s = 0.0842)
								*  Modulation.Cosine( wavelength_s = 0.3541)
								*  Modulation.Linear( intensity = 700)
								- 'Intensity')
							)
                        ),
				Stimulus.Blank( duration_s = 6  ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Synthesis from discrete sinusoids.').setAgenda( agenda )
