from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape ( 
					duration_s = 100,
					shape = Pif.MouseRect(
							
                            size_um = Interactive.MouseMotion(
                            		minimum = (100, 300) ,
                            		maximum = (300, 2000) ,
                            		initialValue = (200, 1000) ,
                            		label = 'Size' ,
                            		key = 'Z' ,
									),
							),
                    shapeMotion = Motion.MouseCrossing(),
			),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


