from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Increasing size red spot with fftBox filter', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 64,
					color='red',
					spatialFilter = Spatial.FftBox()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='red',
					spatialFilter = Spatial.FftBox()
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 256,
					color='red',
					spatialFilter = Spatial.FftBox()
					),
            Stimulus.Spot   (
					duration_s = 20,
					radius = 512,
					color='red',
					spatialFilter = Spatial.FftBox()
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

