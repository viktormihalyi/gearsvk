from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape ( 
					duration_s = 100,
					shape = Pif.MouseRect(),
                    shapeMotion = Motion.MouseCrossing(),
			),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


