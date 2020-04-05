from Project.Components import *

def create(mediaWindow):
    return SpotSequence('Tiny red spot', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 400,
					innerRadius = 300,
					position = (0, 0),
					color='white',
					background=0.5,
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 400,
					innerRadius = 300,
					position = (0, 0),
					color='white',
					background=0.5,
					#spatialFilter = Spatial.IdealLowpass(),
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 64,
					position = (0, 0),
					color='white',
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Spot   (
					duration_s = 1,
					radius = 128,
					color='white',
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
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
					duration_s = 1,
					radius = 256,
					color='white',
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 256,
					color='white',
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 256,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Spot   (
					duration_s = 1,
					radius = 512,
					color='white',
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 512,
					color='white',
					spatialFilter = Spatial.SmallDog()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 512,
					color='white',
					spatialFilter = Spatial.BigDog()
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

