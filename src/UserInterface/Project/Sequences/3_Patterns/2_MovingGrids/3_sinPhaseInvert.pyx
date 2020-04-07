from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Phase reverting sin grating').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
			Stimulus.Blank( 
					duration_s	= 20,
					color	= 0.5,
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
					color	= 0.5,
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

