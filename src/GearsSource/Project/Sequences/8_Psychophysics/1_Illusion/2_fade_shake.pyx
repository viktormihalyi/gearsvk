from Project.Components import *

def fade(image1, image2) :
	return [
			Stimulus.ImageFadeShake    ( 
					duration_s = 4,
					imagePath1=image1,
					imagePath2=image2,
					weightingSlope='hold',
					weighting=0,
					),
			Stimulus.ImageFadeShake    ( 
					duration_s = 4,
					imagePath1=image1,
					imagePath2=image2,
					weightingSlope='up',
					),
			Stimulus.ImageFadeShake    ( 
					duration_s = 4,
					imagePath1=image1,
					imagePath2=image2,
					weightingSlope='hold',
					),			
			]

def create(mediaWindow):
	agenda = [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 1  ),
			]
	agenda += fade( './Project/Media/rotsnakeGDD.jpg',  './Project/Media/rotsnakeG.jpg' )
	agenda += fade( './Project/Media/rotsnakeG.jpg',  './Project/Media/rotsnakeGLL.jpg' )
	agenda += fade( './Project/Media/rotsnakeGLL.jpg',  './Project/Media/rotsnakeG.jpg' )
	agenda += [	
			Stimulus.Image    ( 
					duration_s = 20,
					imagePath='./Project/Media/rotsnakeG.jpg',
					),
			]
	agenda += [
            Stimulus.Blank( duration_s = 1  ),
			EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
			]
	
	return DefaultSequence('Rotating snake').setAgenda( agenda )
