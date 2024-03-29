from Project.Components import *


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=2),
        StartMeasurement(),
        Stimulus.Blank(duration_s=2),
        Stimulus.RandomGrid(
            duration_s=480,
            randomSeed=35436546,
        ),
        EndMeasurement(),
        Stimulus.Blank(duration_s=2),
    ]
    return RandomGridSequence("Random chessboard", frameRateDivisor=1).setAgenda(agenda)
