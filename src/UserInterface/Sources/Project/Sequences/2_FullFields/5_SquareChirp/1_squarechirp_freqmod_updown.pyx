from Project.Components import *


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=2),
        StartMeasurement(),
    ]
    for i in range(0, 3):
        agenda += [
            Stimulus.Blank(duration_s=2),
            Stimulus.FullfieldOscillation(
                duration_s=75,
                wavelength_s=4,
                endWavelength_s=1 / 15,
                exponent=0.01,
            ),
            Stimulus.Blank(duration_s=4.5),
            Stimulus.FullfieldOscillation(
                duration_s=75,
                wavelength_s=1 / 15,
                endWavelength_s=4,
                exponent=0.01,
            ),
            Stimulus.Blank(duration_s=4.5),
        ]
    agenda += [
        EndMeasurement(),
        Stimulus.Blank(duration_s=2),
    ]
    return DefaultSequence(
        "Fullfield with linearly changing modulation frequencies"
    ).setAgenda(agenda)
