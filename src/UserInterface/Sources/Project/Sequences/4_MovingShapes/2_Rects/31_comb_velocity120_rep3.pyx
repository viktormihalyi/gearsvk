from Project.Components import *


class Cross(Stimulus.CrossingRect):
    def boot(self, direction="east"):
        super().boot(
            direction=direction,
            size_um=(1000, 500),
            velocity=120,
            wingmen_distance_um=700,
        )


def create(mediaWindow):
    agenda = [
        Stimulus.Blank(duration_s=1),
        StartMeasurement(),
    ]
    for i in range(0, 3):
        agenda += [
            Cross(direction="east"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="west"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="south"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="north"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="southeast"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="northwest"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="southwest"),
            Stimulus.Blank(duration_s=1),
            Cross(direction="northeast"),
        ]
    agenda += [EndMeasurement(), Stimulus.Blank(duration_s=1)]
    return DefaultSequence("Moving shapes").setAgenda(agenda)
