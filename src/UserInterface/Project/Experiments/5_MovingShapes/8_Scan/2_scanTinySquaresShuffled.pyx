from Project.Components import *
import random

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			Stimulus.Blank( duration_s = 1  ),
			]
	nodes = []
	for j in range(0, 7) :
		for i in range(0, 7) :
			nodes += [(i, j)]
	random.shuffle(nodes)
	for u in range(0, 7*7) :
		agenda += [
				Stimulus.ScanRect(
						duration_s = 1,
						grid = (7, 7),
						index = nodes[u],
						),
				Stimulus.Blank( duration_s = 2  ),
				]
	agenda += [
			Stimulus.Blank( duration_s = 1  ),
			ClearSignal('Exp sync'),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return SquareScanSequence('Scan rectangular field with white squares in random order').setAgenda( agenda )
