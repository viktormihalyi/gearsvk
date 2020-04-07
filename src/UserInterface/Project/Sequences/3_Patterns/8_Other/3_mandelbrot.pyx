from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Mandelbrot set').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape    ( 
					duration_s	=	20,
					pattern		=	Pif.Mandelbrot(
										color1 = Interactive.MouseColor(
											initialValue = 'white',
											label = 'bright color',
											key = 'W'							
										),
										color2 = Interactive.MouseColor(
											initialValue = 'black',
											label = 'dark color',
											key = 'K'							
										),
							),
					),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

