from Project.Components import *


def create(mediaWindow):
    agenda = [
        StartMeasurement(),
        Stimulus.RandomGrid_XorShift128(
            duration_s=480,
            randomSeed=35436546,
        ),
        ClearSignal("Exp sync"),
        EndMeasurement(),
    ]
    return DefaultSequence("Random chessboard", frameRateDivisor=2).setAgenda(agenda)
