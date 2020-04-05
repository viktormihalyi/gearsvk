from Project.Components import *

def create(mediaWindow):
	return DefaultSequence('Tiny red spot').setAgenda( [
			Stimulus.Blank( duration_s = 0.5  )									,
			StartMeasurement()													,
			Stimulus.Blank( duration_s = 0.5 )									,
			Stimulus.Spot   ( duration_s = 2000, radius = 50, color='red' )		,
			Stimulus.Blank( duration_s = 0.5  )									,
            EndMeasurement()													,
			Stimulus.Blank( duration_s = 0.5  )									
		] )


