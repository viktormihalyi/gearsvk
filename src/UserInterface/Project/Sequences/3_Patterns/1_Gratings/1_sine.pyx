from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Simple sine gratings').setAgenda( [
            Stimulus.Blank( duration_s = 1  ),
            StartMeasurement()             ,
            Stimulus.SingleShape    ( 
					duration_s	=	5,
					pattern		=	Pif.SineGrating(
							direction = Interactive.MouseWheel(
									label = 'Direction' ,
									initialValue = 'east' ,
									key = 'D' ,) ,
							color2 = Interactive.MouseColor(
									label = 'Color2' ,
									initialValue = 'black' ,
									key = 'C' ,) ,
							wavelength = Interactive.MouseWheel(
									maximum = 1000 ,
									initialValue = 73 ,
									label = 'Wavelength' ,
									key = 'W' ,									
								) ,
							),
					),
            Stimulus.SingleShape    ( 
					duration_s	=	5,
					pattern		=	Pif.SineGrating(
							wavelength = 100 ,
							direction = Interactive.MouseWheel(
									label = 'Direction' ,
									initialValue = 'east' ,
									key = 'D' ,) ,
							color2 = Interactive.MouseColor(
									label = 'Color2' ,
									initialValue = 'black' ,
									key = 'C' ,) ,
							),
					),					

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  ),
        ] )

