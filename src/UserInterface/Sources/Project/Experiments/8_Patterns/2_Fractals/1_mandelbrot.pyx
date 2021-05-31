from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Mandelbrot set').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Generic    ( 
					duration_s	=	20,
					pattern		=	Pif.Mandelbrot(
							),
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

