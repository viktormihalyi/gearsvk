from Project.Components import *


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=2),
        StartMeasurement(),
    ]
    for i in range(0, 3):
        agenda += [
            Stimulus.Blank(duration_s=1),
            Stimulus.FullfieldGradient(
                duration_s=3,
                direction="east",
            ),
            Stimulus.Blank(duration_s=1),
            Stimulus.FullfieldGradient(
                duration_s=3,
                direction="east",
                start=0,
            ),
            Stimulus.Blank(duration_s=1),
            Stimulus.FullfieldGradient(
                duration_s=3,
                direction="east",
                start=-10.0,
                end=+10.0,
            ),
            Stimulus.Blank(duration_s=1),
            Stimulus.FullfieldGradient(
                duration_s=3,
                direction="northwest",
            ),
            Stimulus.Blank(duration_s=1),
            Stimulus.FullfieldGradient(
                duration_s=3,
                direction="northwest",
                color1="yellow",
                color2="blue",
                start=-300,
                end=+300,
            ),
            Stimulus.Blank(duration_s=1),
        ]
    agenda += [
        EndMeasurement(),
        Stimulus.Blank(duration_s=2),
    ]
    return DefaultSequence("Various fullfield gradients").setAgenda(agenda)
