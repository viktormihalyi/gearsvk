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
                        duration_s = 15,
						spass = Pass.Generic(
							pif = Pif.Solid() 
								<< (Modulation.Cosine( wavelength_s = 1/2)
								*  Modulation.Cosine( wavelength_s = 15)
								- 'Intensity')
							)
                        ),
				Stimulus.SinglePass(
                        duration_s = 15,
						spass = Pass.Generic(
							pif = Pif.Solid() 
								<< (Modulation.Cosine( wavelength_s = 3/2)
								*  Modulation.Cosine( wavelength_s = 3.2/2)
								- 'Intensity')
							)
                        ),
				Stimulus.SinglePass(
                        duration_s = 15,
						spass = Pass.Generic(
							pif = Pif.Solid() 
								<< (Modulation.Cosine( wavelength_s = 10/5	, phase = 1.5 )
								*  Modulation.Cosine( wavelength_s = 10/7	, phase = 0.5 )
								*  Modulation.Cosine( wavelength_s = 10/11	, phase = 2 )
								*  Modulation.Cosine( wavelength_s = 10/13	, phase = 1.5 )
								*  Modulation.Cosine( wavelength_s = 1.5458	, phase = 3 )
								*  Modulation.Cosine( wavelength_s = 3.0045	, phase = 1.5 )
								*  Modulation.Cosine( wavelength_s = 2.4785	, phase = 0 )
								*  Modulation.Cosine( wavelength_s = 0.0842	, phase = 1.5 )
								*  Modulation.Cosine( wavelength_s = 0.3541	, phase = 1 )
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
