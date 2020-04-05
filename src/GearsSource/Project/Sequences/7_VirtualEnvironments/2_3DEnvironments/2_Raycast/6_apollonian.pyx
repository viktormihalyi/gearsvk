from Project.Components import *

def create(mediaWindow):
	sequence = DefaultSequence('Flyover')
	sequence.setAgenda( [
			#Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            #Stimulus.Blank( duration_s = 1  ),
            Stimulus.SinglePass(
					name = 'Apollonian',
					duration_s = 10,
					spass = Pass.ExtApollonian(),
				),
            #Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            #Stimulus.Blank( duration_s = 1  ),
		] )
	return sequence

