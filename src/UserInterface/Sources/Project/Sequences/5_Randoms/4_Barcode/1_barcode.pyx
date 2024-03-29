from Project.Components import *


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=2),
        StartMeasurement(),
        Stimulus.Blank(duration_s=2),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            initialValue="random",
        ),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            step=(1, 0),
            initialValue="keep",
        ),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            randomGridSize=(1, 0),
            step=(0, -1),
            initialValue="random",
        ),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            randomGridSize=(1, 0),
            step=(0, 1),
            initialValue="keep",
        ),
        Stimulus.Blank(duration_s=2),
        ClearSignal("Exp sync"),
        EndMeasurement(),
        Stimulus.Blank(duration_s=2),
    ]
    return DefaultSequence(
        "Random barcode",
        frameRateDivisor=1,
        exportRandomsWithHashmarkComments=True,
        exportRandomsChannelCount=1,
        exportRandomsAsReal=False,
        exportRandomsAsBinary=True,
    ).setAgenda(agenda)
