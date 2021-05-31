from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('White spot with exponential filter', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank( duration_s = 1.0  ),
            StartMeasurement()             ,
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.Exponential(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					temporalFilter = Temporal.Exponential(),
					),
			Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.Exponential(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					temporalFilter = Temporal.Exponential(),
					),
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.Exponential(),
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1.0  ),
        ] )

