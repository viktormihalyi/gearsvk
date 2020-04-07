from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Resizable red spot').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot    ( 
					duration_s = 2000,
					radius = Interactive.MouseWheel(
							label = 'radius',
							initialValue = 100,
							minimum = 0,
							maximum = 1400,
							key = 'R',
							),
					innerRadius = Interactive.MouseWheel(
							label = 'radius',
							initialValue = 0,
							minimum = 0,
							maximum = 1400,
							key = 'H',
							),
					color='red',
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

