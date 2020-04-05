from Project.Components import *

def create(mediaWindow):
	agenda = [
			#Stimulus.Blank( duration_s = 1  ),
			StartMeasurement()             ,
			]
	filterControl = Spatial.DogFftSpatSpec(
					sigma1 = Interactive.MouseWheel(
							label = 'filter sigma',
							initialValue = 100,
							minimum = 1,
							maximum = 200,
							key = 'O',
							),
					weight1 = 10,
					weight2 = 0,
					)
	agenda += [
		Stimulus.Video    ( 
			duration_s = 360,
			videoPath = './Project/Media/Birds_7.mp4',
			spatialFilter = filterControl,
			),
		]
	agenda += [
		EndMeasurement(),
		Stimulus.Blank( duration_s = 1  )
		]
	return DefaultSequence('Video-to-texture test').setAgenda( agenda )

