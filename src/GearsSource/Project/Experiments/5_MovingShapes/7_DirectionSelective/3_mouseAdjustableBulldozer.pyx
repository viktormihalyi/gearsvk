from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Generic ( 
					duration_s = 100,
					shape = Shape.MouseRect(
                            size_um = (200, 6000)),
                    shapeMotion = Motion.MouseCrossing(),
			),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


