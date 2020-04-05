from Project.Components import *
import math

class Cross (Stimulus.SingleShape) :
    def boot(self, direction='east', radius = 500, speed=1200):   
        angle = processDirection(direction, None)
        super().boot( 
		    duration_s = radius * 2 / speed,
		    shape = Pif.Rect(
	            facing = angle,
        	    size_um = (200, 20000),
			),
		    shapeMotion = Motion.Linear(
	                velocity = (speed * math.cos(angle), speed * math.sin(angle)),
					startPosition = (-radius * math.cos(angle), -radius * math.sin(angle)),
			),
		)

def create(mediaWindow):
    return DefaultSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Cross( direction = 'east' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'west' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'south' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'north' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'southeast' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'northwest' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'southwest' ),
            Stimulus.Blank( duration_s = 1  ),
            Cross( direction = 'northeast' ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


