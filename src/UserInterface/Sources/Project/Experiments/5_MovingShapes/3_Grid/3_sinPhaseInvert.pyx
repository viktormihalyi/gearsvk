from Project.Components import *

def create(mediaWindow):
    return MovingShapeSequence('Moving sin grating').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
			Stimulus.Blank( 
					duration_s	= 20,
					intensity	= 0.5,
					),
            Stimulus.PhaseInvertingGrid    ( 
					duration_s	=	4,
					direction	=	'east',
					),
            Stimulus.PhaseInvertingGrid    ( 
					duration_s	=	4,
					direction	=	'north',
					),
			Stimulus.Blank( 
					duration_s	= 2,
					intensity	= 0.5,
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

