from Project.Components import *

class Cross (Stimulus.CrossingRect) :
    def boot(self, direction='east', offset_um=0):
        super().boot( 
                direction = direction,
                size_um = (1000, 500),
                velocity = 1200,
				offset_um = offset_um
				)

def scan(direction, span=2000, count=7):
	crossList = []
	for i in range(0, count):
		crossList += [Cross(
				direction = direction,
				offset_um = -span * 0.5 + i * span / (count - 1)
				)]
	return crossList

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 1  ),
			StartMeasurement()             ,
			]
	for i in range(0, 6) :
		agenda	+=	scan( direction = 'east' )
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'west' )
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'south' )
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'north' )
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'southeast', span=2500, count = 8)
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'northwest', span=2500, count = 8 )
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'southwest', span=2500, count = 8 )
		agenda	+=	[Stimulus.Blank( duration_s = 1  )]
		agenda	+=	scan( direction = 'northeast', span=2500, count = 8 )
	agenda += [

            EndMeasurement(),
			Stimulus.Blank( duration_s = 1 ),
			]
	return MovingShapeSequence('Moving shapes').setAgenda( agenda )
	 
            



