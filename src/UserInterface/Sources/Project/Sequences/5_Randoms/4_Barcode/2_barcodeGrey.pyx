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
            shape=Pif.GreyscaleRandomGrid(),
        ),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            step=(1, 0),
            initialValue="keep",
            shape=Pif.GreyscaleRandomGrid(),
        ),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            randomGridSize=(1, 0),
            step=(0, -1),
            initialValue="random",
            shape=Pif.GreyscaleRandomGrid(),
        ),
        Stimulus.ShiftingBarcode(
            duration_s=3,
            randomSeed=35436546,
            randomGridSize=(1, 0),
            step=(0, 1),
            initialValue="keep",
            shape=Pif.GreyscaleRandomGrid(),
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
        exportRandomsAsReal=True,
        exportRandomsAsBinary=False,
    ).setAgenda(agenda)
