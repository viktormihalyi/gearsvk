from Project.Components import *

def create(mediaWindow):
    return PolychromeSequence('Polychrome prototype', stimulusWindow=mediaWindow.stimulusWindow).setAgenda( [
            BlackoutWithPolychromePreparationStimulus( duration_s = 1  ),
            StartMeasurement()             ,
            BlackoutWithPolychromePreparationStimulus( duration_s = 1, wavelength = 400  ),
            PolychromeStimulus    ( duration_s = 3, wavelength = 400 ),
            BlackoutWithPolychromePreparationStimulus( duration_s = 1, wavelength = 500  ),
            PolychromeStimulus    ( duration_s = 3, wavelength = 500 ),
            BlackoutWithPolychromePreparationStimulus( duration_s = 1, wavelength = 600  ),
            PolychromeStimulus    ( duration_s = 3, wavelength = 600 ),
            BlackoutWithPolychromePreparationStimulus( duration_s = 1 , wavelength = 690 ),
            PolychromeStimulus    ( duration_s = 3, wavelength = 690 ),
            Stimulus.Blank( duration_s = 1  ),

            EndMeasurement(),
            Stimulus.Blank( duration_s = 1  )
        ] )


