from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape ( 
					duration_s = 100,
					shape = Pif.MouseRect(
							size_um = Interactive.MouseMotion(
									label = 'Size',
									key = 'K',
									initialValue = (2000,200),
									minimum = (200,100),
									maximum = (2500,300.),
								),
							),
                    shapeMotion = Motion.MouseCrossing(),
			),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


