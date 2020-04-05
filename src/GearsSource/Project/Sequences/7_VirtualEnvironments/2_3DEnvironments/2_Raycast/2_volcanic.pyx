from Project.Components import *

def create(mediaWindow):
	sequence = DefaultSequence('Flyover')
	sequence.setAgenda( [
			#Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            #Stimulus.Blank( duration_s = 1  ),
			Stimulus.SinglePass(
					name = 'Volcanic',
					duration_s = 10,
					spass = Pass.ExtVolcanic(),
				),
            #Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            #Stimulus.Blank( duration_s = 1  ),
		] )
	return sequence

