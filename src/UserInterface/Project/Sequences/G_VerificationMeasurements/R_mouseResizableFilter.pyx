from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Resizable filter').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.SingleShape    (
            		duration_s = 60,
					shape = Pif.Spot    (
						radius = Interactive.MouseWheel(
								label = 'radius',
								initialValue = 800,
								minimum = 0,
								maximum = 1400,
								key = 'R',
								),
						innerRadius = Interactive.MouseWheel(
								label = 'hole',
								initialValue = 200,
								minimum = 0,
								maximum = 1400,
								key = 'H',
								),
						),
					spatialFilter = Spatial.DogFftSpatSpec(
						sigma1 = Interactive.MouseWheel(
								label = 'filter sigma',
								initialValue = 100,
								minimum = 1,
								maximum = 1000,
								key = 'O',
								),
						weight1 = 10,
						weight2 = 0,
						),
					),
			Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

