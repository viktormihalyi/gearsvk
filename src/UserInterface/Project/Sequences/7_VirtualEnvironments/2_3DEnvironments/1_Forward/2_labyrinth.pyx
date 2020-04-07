from Project.Components import *

def create(mediaWindow):
	sequence = DefaultSequence('Labyrinth')
	sequence.setAgenda( [
			#Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            #Stimulus.Blank( duration_s = 1  ),
            Stimulus.SingleShape(
					duration_s = 10,
					forward = Forward.Labyrinth( ),
					pattern = Pif.ForwardRendered( ),
				),
            #Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            #Stimulus.Blank( duration_s = 1  ),
		] )
	return sequence

