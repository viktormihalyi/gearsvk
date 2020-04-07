from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 2  ),
			]
	for i in range(0, 3) :
		agenda += [
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=220, sigma2=20, weight2=-220, offset=1), radius = 62.5 ),
				Stimulus.Blank( duration_s = 6.5, color=0.4  ),
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=220, sigma2=20, weight2=-220, offset=1), radius = 125 ),
				Stimulus.Blank( duration_s = 6.5, color=0.4  ),
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=220, sigma2=20, weight2=-220, offset=1), radius = 187.5 ),
				Stimulus.Blank( duration_s = 6.5, color=0.4  ),
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=220, sigma2=20, weight2=-220, offset=1), radius = 250 ),
				Stimulus.Blank( duration_s = 6.5, color=0.4 ),
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=220, sigma2=20, weight2=-220, offset=1), radius = 312.5 ),
				Stimulus.Blank( duration_s = 6.5, color=0.4  ),
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=220, sigma2=20, weight2=-220, offset=1), radius = 5000 ),
				Stimulus.Blank( duration_s = 6.5, color=0.4 ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2, color=0.4  ),
			]
	return DefaultSequence('Spots with increasing radii').setAgenda( agenda )

