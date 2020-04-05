from Project.Components import *

def create(mediaWindow):
    return CampbellRobertsonSequence('Campbell-Robertson contrast sensitivity chart').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.CampbellRobertson    ( 
					duration_s	=	30,
					direction	=	'east',
					startWavelength = 30,
					endWavelength = 30,
					),
            Stimulus.CampbellRobertson    ( 
					duration_s	=	30,
					direction	=	'west',
					startWavelength = 30,
					endWavelength = 30,
					),
			Stimulus.CampbellRobertson    ( 
					duration_s	=	30,
					direction	=	'east',
					maxContrast	=	0.5,
					startWavelength = 30,
					endWavelength = 30,
					),
            Stimulus.CampbellRobertson    ( 
					duration_s	=	30,
					direction	=	'west',
					maxContrast	=	0.5,
					startWavelength = 30,
					endWavelength = 30,
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

