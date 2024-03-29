from Project.Components import *


def create(mediaWindow):
    return DefaultSequence("Campbell-Robertson contrast sensitivity chart").setAgenda(
        [
            Stimulus.Blank(duration_s=1),
            StartMeasurement(),
            Stimulus.CampbellRobertson(
                duration_s=3,
                direction="east",
                startWavelength=30,
                endWavelength=30,
            ),
            Stimulus.CampbellRobertson(
                duration_s=3,
                direction="west",
                startWavelength=30,
                endWavelength=30,
            ),
            Stimulus.CampbellRobertson(
                duration_s=3,
                direction="east",
                maxContrast=0.5,
                startWavelength=30,
                endWavelength=30,
            ),
            Stimulus.CampbellRobertson(
                duration_s=3,
                direction="west",
                maxContrast=0.5,
                startWavelength=30,
                endWavelength=30,
            ),
            EndMeasurement(),
            Stimulus.Blank(duration_s=1),
        ]
    )
