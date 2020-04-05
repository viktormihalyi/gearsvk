from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Changeble color').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.SingleShape    ( 
					duration_s = 60,
					shape = Pif.Rect(
						size_um = Interactive.MouseMotion(
								label = 'rect size',
								initialValue = (100, 100),
								minimum = (0, 0),
								maximum = (1400, 1400),
								key = 'D',
								),
						),
					pattern = Pif.Solid(
						color = Interactive.MouseColor(
								label = 'Shape color',
								key = 'K',
								),
						),
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

