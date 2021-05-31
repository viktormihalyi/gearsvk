from Project.Components import *

class Cross (Stimulus.CrossingRect) :
    def boot(self, direction='east'):
        super().boot( 
                direction = direction,
                size_um = (300, 20000),
                velocity = 1200,
				)

def create(mediaWindow):
    return MovingShapeSequence('Moving shapes').setAgenda( [
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


