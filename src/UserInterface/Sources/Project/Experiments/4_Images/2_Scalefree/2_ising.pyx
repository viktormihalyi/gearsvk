from Project.Components import *


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=1),
        StartMeasurement(),
        Stimulus.Blank(duration_s=1),
        Stimulus.Image(
            duration_s=40,
            imagePath="./Project/Media/ising.png",
            velocity=(200, 200),
        ),
        Stimulus.Blank(duration_s=1),
        EndMeasurement(),
        Stimulus.Blank(duration_s=1),
    ]

    return ImageSequence("Rotating snake").setAgenda(agenda)
