from Project.Components import *

def create(mediaWindow):
    return ImageSequence('Rotating snake').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.Image    ( 
					duration_s = 10,
					imagePath='./Project/Media/intens.jpg',
					),
			Stimulus.Blank( duration_s = 4  ),
			Stimulus.ImageFade    ( 
					duration_s = 3,
					imagePath1='./Project/Media/intens.jpg',
					imagePath2='./Project/Media/greyonly.jpg',
					weightingSlope='hold',
					weighting=0,
					),
			Stimulus.ImageFade    ( 
					duration_s = 4,
					imagePath1='./Project/Media/intens.jpg',
					imagePath2='./Project/Media/greyonly.jpg',
					weightingSlope='up',
					),
			Stimulus.ImageFade    ( 
					duration_s = 3,
					imagePath1='./Project/Media/intens.jpg',
					imagePath2='./Project/Media/greyonly.jpg',
					weightingSlope='hold',
					),
			Stimulus.Blank( duration_s = 4 , intensity=0.5 ),
			Stimulus.ImageFade    ( 
					duration_s = 3,
					imagePath1='./Project/Media/intens.jpg',
					imagePath2='./Project/Media/greyonly.jpg',
					weightingSlope='hold',
					),
			Stimulus.ImageFade    ( 
					duration_s = 4,
					imagePath1='./Project/Media/intens.jpg',
					imagePath2='./Project/Media/greyonly.jpg',
					weightingSlope='down',
					),
			Stimulus.ImageFade    ( 
					duration_s = 3,
					imagePath1='./Project/Media/intens.jpg',
					imagePath2='./Project/Media/greyonly.jpg',
					weightingSlope='hold',
					weighting=0,
					),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])

