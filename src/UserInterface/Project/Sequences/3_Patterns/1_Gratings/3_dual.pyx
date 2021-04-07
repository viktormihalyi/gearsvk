from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Simple sine gratings').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Generic ( 
					duration_s	=	20,
					passes = [
						Pass.Generic(
								pif		=	Pif.SineGrating(
										color1		=	'blue'
								),
						),
						Pass.Generic(
								pif		=	Pif.SineWheel(
										color1		=	'green',
								),
						),
						],
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

