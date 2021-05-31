from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Campbell-Robertson contrast sensitivity chart').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.CampbellRobertson    (
            		duration_s	=	30,
					direction	=	'east',
					
					),
		
            Stimulus.CampbellRobertson    (
            		duration_s	=	30,
					direction	=	'west',
					),


			Stimulus.CampbellRobertson    ( 
					duration_s	=	30,
					direction	=	'east',
					contrast	=	(0, 0.5),
					),
            Stimulus.CampbellRobertson    ( 
					duration_s	=	30,
					direction	=	'west',
					
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

