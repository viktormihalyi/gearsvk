from Project.Components import *

def create(mediaWindow):
	sequence = DefaultSequence('Perspective flyby')
	sequence.setAgenda( [
			Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 1  ),
            Stimulus.SingleShape(
					duration_s = 20,
					forward = Forward.Flyby( ),
					pattern = Pif.ForwardRendered( ),
				),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
		] )
	return sequence

