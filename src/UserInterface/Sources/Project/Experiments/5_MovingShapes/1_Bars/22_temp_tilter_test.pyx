from Project.Components import *

class Cross (Stimulus.CrossingRect) :
    def boot(self, direction='east'):
        super().boot( 
                direction = direction,
                size_um = (200, 20000),
                velocity = 1800,
				temporalFilter=Temporal.Exponential(),
				)

def create(mediaWindow):
    return MovingShapeSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Cross( direction = 'east' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'west' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'south' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'north' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'southeast' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'northwest' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'southwest' ),
            Stimulus.Blank( duration_s = 1 , temporalFilter=Temporal.Exponential(), ),
            Cross( direction = 'northeast' ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


