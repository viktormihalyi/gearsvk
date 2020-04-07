from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('White/grey spot with exponential LTI filter', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank(
					duration_s = 1.0,
					temporalFilter = Temporal.ExponentialLti(),
					),
            StartMeasurement()             ,
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.ExponentialLti(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					temporalFilter = Temporal.ExponentialLti(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color=(0.5, 0.5, 0.5),
					temporalFilter = Temporal.ExponentialLti(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					temporalFilter = Temporal.ExponentialLti(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color=(0.5, 0.5, 0.5),
					temporalFilter = Temporal.ExponentialLti(),
					),
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.ExponentialLti(),
					),
            EndMeasurement(),
            Stimulus.Blank( 
					duration_s = 1.0,
					temporalFilter = Temporal.ExponentialLti(),
					),
        ] )

