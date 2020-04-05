from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('White spot with bimodal temporal filter', frameRateDivisor=1).setAgenda( [
            StartMeasurement(),
			Stimulus.Blank(
					duration_s = 1 ,),
			Response.Start(loop = True),
			Stimulus.Spot   (
					color = 'white' ,
					radius = 500 ,
					duration_s = 5 ,),
			Stimulus.Blank(
					duration_s = 5 ,),
			Response.End(),
			Stimulus.Blank(
					duration_s = 1 ,),
			Stimulus.Spot   (
					color = 'red' ,
					radius = 500 ,
					duration_s = 5 ,),
			Stimulus.Blank(
					duration_s = 5 ,),
			EndMeasurement(),
            Stimulus.Blank(),
        ] )

