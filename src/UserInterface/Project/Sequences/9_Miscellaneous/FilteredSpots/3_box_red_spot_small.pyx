from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Increasing size red spot with SmallDog filter', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 64,
					color='white',
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 256,
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Spot   (
            		color = 'green' ,
					duration_s = 20,
					radius = 512,
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

