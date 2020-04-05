from Project.Components import *

def create(mediaWindow):
	sequence = DefaultSequence('Perspective flyover')
	sequence.setAgenda( [
			Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 1  ),
			Stimulus.SinglePass(
					name = 'Cloudy terrain',
					duration_s = 30,
					spass = Pass.ExtCloudyTerrain(),
				),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
		] )
	return sequence

