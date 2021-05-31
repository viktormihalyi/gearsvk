from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Colored spots with bimodal LTI filter (3 states)', frameRateDivisor=1).setAgenda( [
            Stimulus.Blank(
					duration_s = 1.0,
					temporalFilter = Temporal.CellLti3(),
					),
            StartMeasurement()             ,
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.CellLti3(),
					),
            Stimulus.Spot   (
					duration_s = 4,
					radius = 128,
					color=(1.0, 0.0, 1.0),
					temporalFilter = Temporal.CellLti3(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color=(0.0, 0.5, 0.5),
					temporalFilter = Temporal.CellLti3(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color='yellow',
					temporalFilter = Temporal.CellLti3(),
					),
            Stimulus.Spot   (
					duration_s = 2,
					radius = 128,
					color=(0.5, 0.0, 0.5),
					temporalFilter = Temporal.CellLti3(),
					),
            Stimulus.Blank( 
					duration_s = 2.0,
					temporalFilter = Temporal.CellLti3(),
					),
            EndMeasurement(),
            Stimulus.Blank( 
					duration_s = 1.0,
					temporalFilter = Temporal.CellLti3(),
					),
        ] )

