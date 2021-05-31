from Project.Components import *

def create(mediaWindow):
	return DefaultSequence('Tiny red spot').setAgenda( [
			Stimulus.Blank( duration = 2  )									,
			StartMeasurement()													,
			Stimulus.Spot   ( duration = 6, radius = 50, color='red' )		,
            EndMeasurement()													,
			Stimulus.Blank( duration = 2  )									
		] )


