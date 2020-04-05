from Project.Components import *

def create(mediaWindow):
    return SpotSequence('Tiny red spot', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank(
					duration_s = 1.0,
					temporalFilter = Temporal.Cell(),
					),
            StartMeasurement()             ,
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.Cell(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					temporalFilter = Temporal.Cell(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color=(0.5, 0.5, 0.5),
					temporalFilter = Temporal.Cell(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='white',
					temporalFilter = Temporal.Cell(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color=(0.5, 0.5, 0.5),
					temporalFilter = Temporal.Cell(),
					),
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.Cell(),
					),
            EndMeasurement(),
            Stimulus.Blank( 
					duration_s = 1.0,
					temporalFilter = Temporal.Cell(),
					),
        ] )

