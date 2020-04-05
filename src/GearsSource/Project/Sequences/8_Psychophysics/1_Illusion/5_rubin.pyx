from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Rubin vase').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
			Response.Start(question = "What do you see?",
							buttons = [ ('Vase', -500, -600, 400, 100, 'V', True),
										('Faces', 500, -600, 400, 100, 'F', True),],
										loop = True, ),
            Stimulus.SinglePass    (
            		spass = Pass.Generic(
							pif = Pif.Image(
									imagePath='./Project/Media/rubin.jpg',
									)
									** ( Pif.Solid(color=0), (
										Pif.Image(imagePath='./Project/Media/eye.png',)<<Warp.Clamp()<<Motion.Linear(
											positionUnits = 'percent',
											startPosition = (-0.2, -0.1) ,
											startScale = (0.1, 0.1) , )) 
										* (Pif.Image(imagePath='./Project/Media/eye.png',)<<Warp.Clamp()<<Motion.Linear(
											positionUnits = 'percent',
											startPosition = (+0.2, -0.1) ,
											startScale = (-0.1, 0.1) , )), )
							), 
					duration_s = 5,
					),
			Response.End(),
			Response.Start(question = "What do you see?",
							buttons = [ ('Vase', -500, -600, 400, 100, 'V', True),
										('Faces', 500, -600, 400, 100, 'F', True),],
										loop = True, ),
            Stimulus.SinglePass    (
            		spass = Pass.Generic(
            				pif = Pif.Image(
									imagePath='./Project/Media/rubin.jpg',
									)
									** (Pif.Solid(color=0),  Pif.Solid(color=1)  ),
							), 
					duration_s = 5,
					),
			Response.End(),
            EndMeasurement()             ,
            Stimulus.Blank( duration_s = 1  )
        ])