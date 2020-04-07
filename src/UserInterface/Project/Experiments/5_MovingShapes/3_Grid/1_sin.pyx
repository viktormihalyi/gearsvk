from Project.Components import *

def create(mediaWindow):
    return MovingShapeSequence('Moving sin grating').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
			Stimulus.Blank( 
					duration_s	= 20,
					intensity	= 0.5,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'east',
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'east',
					velocity	=	1200,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'east',
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'east',
					velocity	=	-1200,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'east',
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'north',
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'north',
					velocity	=	1200,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'north',
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'north',
					velocity	=	-1200,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'north',
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southeast',
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southeast',
					velocity	=	1200,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southeast',
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southeast',
					velocity	=	-1200,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southeast',
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southwest',
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southwest',
					velocity	=	1200,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southwest',
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southwest',
					velocity	=	-1200,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southwest',
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

