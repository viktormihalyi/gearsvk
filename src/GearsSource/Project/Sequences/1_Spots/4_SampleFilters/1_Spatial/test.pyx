from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Increasing size white spot/annuli with different Dog filters', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot   (
            		duration_s = 2,
					radius = 128,
					position = (0, 0),
					color='white',
					spatialFilter = Spatial.SmallDog()
					),
            
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					spatialFilter = Spatial.DogFftFreqSpec()
					),

            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					spatialFilter = Spatial.DogFftSpatSpec(
							sigma2 = 5.0 ,
							sigma1 = 10.0 ,
							weight2 = -10.0 ,
							weight1 = 10.0,
							)
					),

            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

