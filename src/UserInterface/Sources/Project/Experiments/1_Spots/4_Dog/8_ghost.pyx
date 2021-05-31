from Project.Components import *

def create(mediaWindow):
    return SpotSequence('Tiny red spot', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot   (
					duration_s = 1,
					radius = 300,
					position = (0, 0),
					color='white',
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 300,
					position = (300, 300),
					color='white',
					#spatialFilter = Spatial.IdealLowpass(),
					spatialFilter = Spatial.BigDog(),
					#spatialFilter = Spatial.SmallDog(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 300,
					position = (0, 0),
					color='white',
					#spatialFilter = Spatial.IdealLowpass2(),
					#spatialFilter = Spatial.BigDog(),
					spatialFilter = Spatial.SmallDog(),
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

