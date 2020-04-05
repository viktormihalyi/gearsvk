from Project.Components import *

def create(mediaWindow):
    return SpotSequence('Tiny red spot', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 1.0  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 1.0  ),
            Stimulus.Spot   (
					duration_s = 4,
					radius = 64,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Spot   (
					duration_s = 4,
					radius = 128,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Spot   (
					duration_s = 4,
					radius = 256,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Spot   (
					duration_s = 20,
					radius = 512,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

