from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Resizable rectangle').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.SingleShape    ( 
					duration_s = 60,
					shape = Pif.Rect(
						size_um = (
								Interactive.MouseWheel(
									label = 'rect width',
									initialValue = 100,
									minimum = 0,
									maximum = 1400,
									key = 'D',
									),
								Interactive.MouseWheel(
									label = 'rect height',
									initialValue = 100,
									minimum = 0,
									maximum = 1400,
									key = 'F',
									),									
								),
						),
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

