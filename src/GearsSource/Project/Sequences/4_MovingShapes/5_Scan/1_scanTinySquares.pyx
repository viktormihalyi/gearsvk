from Project.Components import *

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 1  ),
			]
	for j in range(0, 7) :
		for i in range(0, 7) :
			agenda += [
					Stimulus.ScanRect(
							duration_s = 1,
							grid = (7, 7),
							index = (i, j),
							),
					Stimulus.Blank( duration_s = 2  ),
					]
	agenda += [
			Stimulus.Blank( duration_s = 1  ),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Scan rectangular field with white squares').setAgenda( agenda )
