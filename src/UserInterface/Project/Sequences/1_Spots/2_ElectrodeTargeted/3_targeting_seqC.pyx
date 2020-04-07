from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 1  ),
			StartMeasurement()             ,
            Stimulus.Blank( duration_s = 3  ),
            Stimulus.Fullfield( duration_s = 2  ),
            Stimulus.Blank( duration_s = 6.5  ),
            Stimulus.Fullfield( duration_s = 2  ),
            Stimulus.Blank( duration_s = 6.5  ),
			]
	for i in range(0, 3) :
		agenda += [
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('O', 3),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('O', 15),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('P', 5),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('R', 6),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('N', 8),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
				]
	agenda += [
            Stimulus.Fullfield( duration_s = 2  ),
            Stimulus.Blank( duration_s = 6.5  ),
            Stimulus.Fullfield( duration_s = 2  ),
            Stimulus.Blank( duration_s = 6.5  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
			]
	return DefaultSequence('Targeting').setAgenda( agenda )
