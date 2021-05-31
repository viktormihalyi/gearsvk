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
                        position    = ('R', 14),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('D', 11),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('I', 7),
                        ),
                Stimulus.Blank( duration_s = 6.5  ),
                Stimulus.Spot(  
                        duration_s  = 2,
                        radius      = 150,
                        position    = ('O', 10),
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
