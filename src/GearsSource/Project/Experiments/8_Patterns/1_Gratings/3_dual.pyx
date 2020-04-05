from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Simple sine gratings').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.MultiPass ( 
					duration_s	=	2,
					passes = [
						spass.Generic(
								pattern		=	Pif.SineWheel(
										color1		=	'green',
								),
						),
						spass.Generic(
								pattern		=	Pif.SineGrating(
										color1		=	'blue'
								),
						),
						],
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

