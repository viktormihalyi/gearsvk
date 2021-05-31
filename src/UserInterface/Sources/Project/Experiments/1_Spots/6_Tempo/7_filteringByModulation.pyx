from Project.Components import *

def create(mediaWindow):
	return SpotSequence('Fast temporal filtering with modulation', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 1.0  ),
            StartMeasurement()             ,
            Stimulus.Blank( 
					duration_s = 1.0,
					),
            Stimulus.Spot   (
					duration_s = 4,
					radius = 128,
					color='white',
					modulation = Modulation.BinaryWithTemporalWeighting(
                            switches = [-2, -1],
							brightColor = 'white', #(0.75, 0.75, 0.75),
							darkColor	= 'black', #(0.25, 0.25, 0.25),
							weighting = Temporal.Cell(),
							),
					),
			Stimulus.Blank( 
					duration_s = 1.0,
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1.0  ),
		] )

