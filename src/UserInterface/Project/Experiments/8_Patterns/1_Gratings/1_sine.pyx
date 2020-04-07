from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Simple sine gratings').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape    ( 
					duration_s	=	2,
					pattern		=	Pif.SineGrating(
							),
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

