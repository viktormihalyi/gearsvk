from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 0.5  ),
			StartMeasurement()             ,
			]
	for i in range(0, 5) :
		agenda += [
				Stimulus.Spot    ( duration_s = 2, color=0.5, background=0.4, radius = 125 ),
				Stimulus.Spot    ( duration_s = 2, color=0.75, background=0.25, spatialFilter=Spatial.DisplayStimFft(), radius = 125 ),
				#Stimulus.Spot    ( duration_s = 2, color=0.75, background=0.25, spatialFilter=Spatial.DogFftFreqSpec(sigma1=10, weight1=-0.30, weight2=0), radius=125 ),
				#Stimulus.Spot    ( duration_s = 2, color=0.75, background=0.25, spatialFilter=Spatial.DogFftFreqSpec(sigma1=1/2/3.14/100, weight1=-0.10, weight2=0), radius = 125 ),
				#Stimulus.Spot    ( duration_s = 2, position=(0,-0), color=0.75, background=0.25, spatialFilter=Spatial.DogFftSpatSpec(sigma1=15, weight1=-200, sigma2=30, weight2=200), radius = 125 ),
				#Stimulus.Spot    ( duration_s = 2, position=(0,-0), color=0.75, background=0.25, spatialFilter=Spatial.DogFftFreqSpec(sigma1=15, weight1=-200, sigma2=30, weight2=200), radius = 125 ),
				Stimulus.Spot    ( duration_s = 2, position=(0,-0), color=0.5, background=0.4, spatialFilter=Spatial.DogFftFreqSpec(sigma1=15, weight1=220, sigma2=30, weight2=-220, offset=1), radius = 500 ),
				]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.5  ),
			]
	sequence = DefaultSequence('Spots with increasing radii')
	sequence.fft_width_px = 1024
	sequence.fft_height_px = 1024
	sequence.setAgenda( agenda )
	return sequence

