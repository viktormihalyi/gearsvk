from Project.Components import *

def create(mediaWindow):
    return MovingShapeSequence('Moving square grating').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
			Stimulus.Blank( 
					duration_s	= 20,
					intensity	= 0.5,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'east',
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'east',
					velocity	=	1200,
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'east',
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'east',
					velocity	=	-1200,
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'east',
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'north',
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'north',
					velocity	=	1200,
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'north',
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'north',
					velocity	=	-1200,
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'north',
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southeast',
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southeast',
					velocity	=	1200,
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southeast',
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southeast',
					velocity	=	-1200,
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southeast',
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southwest',
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southwest',
					velocity	=	1200,
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southwest',
					sineExponent = 0,
					),
            Stimulus.MovingGrid    ( 
					duration_s	=	18,
					direction	=	'southwest',
					velocity	=	-1200,
					sineExponent = 0,
					),
			Stimulus.MovingGrid    ( 
					duration_s	=	1,
					direction	=	'southwest',
					sineExponent = 0,
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

