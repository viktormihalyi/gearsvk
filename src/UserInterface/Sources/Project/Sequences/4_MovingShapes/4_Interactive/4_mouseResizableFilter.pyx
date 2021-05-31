from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Resizable filter').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.SingleShape    (
            		spatialFilter = Spatial.DogFftFreqSpec(
						sigma2 = 5.0 ,
						sigma1 = Interactive.MouseWheel(
								label = 'filter sigma',
								initialValue = 10,
								minimum = 1,
								maximum = 1000,
								key = 'O',
								),
						weight1 = -100,
						weight2 = 0,
						),
            		#pattern = Pif.Solid(
            		#		color = 'green' ,) , 
					duration_s = 60,
					shape = Pif.Rect(
						size_um = Interactive.MouseMotion(
								label = 'rect size',
								initialValue = (100, 100),
								minimum = (0, 0),
								maximum = (1400, 1400),
								key = 'D',
								),
						),
					),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

