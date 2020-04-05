from Project.Components import *

def create(mediaWindow):
    return DefaultSequence('Electrode grid').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.SpotGrid  ( duration_s = 2000, color='red',  ),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

