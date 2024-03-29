from Project.Components import *


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=2),
        StartMeasurement(),
        Stimulus.Blank(duration_s=2),
        Stimulus.SinglePass(
            duration_s=480,
            spass=Pass.Generic(
                pif=Pif.Spot(
                    radius=500,
                )
                ** (
                    Pif.Spot(radius=5000, innerRadius=300)
                    ** (
                        Pif.Solid(color="grey"),
                        Pif.CampbellRobertson(
                            startWavelength=150,
                            endWavelength=150,
                            sineExponent=1,
                            minContrast=1,
                        )
                        << Motion.DiscreteLinear(
                            jump=(150 * 0.5, 0),
                        ),
                    ),
                    Pif.CampbellRobertson(
                        startWavelength=250,
                        endWavelength=250,
                        sineExponent=0.01,
                        minContrast=1,
                    ),
                ),
            ),
        ),
        ClearSignal("Exp sync"),
        EndMeasurement(),
        Stimulus.Blank(duration_s=2),
    ]
    return DefaultSequence(
        "Random chessboard masked by annulus", frameRateDivisor=1
    ).setAgenda(agenda)
