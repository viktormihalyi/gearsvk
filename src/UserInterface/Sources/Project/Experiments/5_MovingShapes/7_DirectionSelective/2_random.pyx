from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Moving shapes').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Generic ( 
					duration_s = 100,
					prng = Prng.XorShift128(
							randomGridSize = (1, 1),
					),
					shape = Shape.RandomRect(),
			),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1 )
        ] )


