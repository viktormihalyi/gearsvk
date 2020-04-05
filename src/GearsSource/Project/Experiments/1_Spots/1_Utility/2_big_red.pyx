from Project.Components import *

def create(mediaWindow):
    return SpotSequence('Tiny red spot').setAgenda( [
            Stimulus.Blank( duration_s = 0.1  ),
            StartMeasurement()             ,
            Stimulus.Blank( duration_s = 0.1  ),
            Stimulus.Spot    ( duration_s = 2000, radius = 800, color='red' ),
            Stimulus.Blank( duration_s = 0.1  ),
            EndMeasurement(),
            Stimulus.Blank( duration_s = 0.1  ),
        ] )

