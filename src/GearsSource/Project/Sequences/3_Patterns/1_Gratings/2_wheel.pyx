from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Simple sine gratings').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape    ( 
					duration_s	=	60,
					pattern		=	Pif.SineWheel(
							wavelength_radians = Interactive.MouseWheel(
									initialValue = 0.39 ,
									label = 'wavelength' ,
									key = 'W' ,),
							direction = Interactive.MouseWheel(
								label = 'Direction',
								key = 'D',
								) ,
							),
					),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

