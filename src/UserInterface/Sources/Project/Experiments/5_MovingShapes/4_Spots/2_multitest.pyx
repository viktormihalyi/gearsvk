from Project.Components import *
import random


def addSpots(sequence, spots, radius, level):
    if level == 0:
        return
    spots += [
        Shape.MultiSpot.Spot(
            position=(
                random.uniform(
                    a=-sequence.field_width_um / 2, b=sequence.field_width_um / 2
                ),
                random.uniform(
                    a=-sequence.field_height_um / 2, b=sequence.field_height_um / 2
                ),
            ),
            velocity=(random.uniform(a=-200, b=200), random.uniform(a=-200, b=200)),
            radius=radius,
            color="white",
        ),
        Shape.MultiSpot.Spot(
            position=(
                random.uniform(
                    a=-sequence.field_width_um / 2, b=sequence.field_width_um / 2
                ),
                random.uniform(
                    a=-sequence.field_height_um / 2, b=sequence.field_height_um / 2
                ),
            ),
            velocity=(random.uniform(a=-200, b=200), random.uniform(a=-200, b=200)),
            radius=radius,
            color="black",
        ),
    ]
    addSpots(sequence, spots, radius / 1.41, level - 1)
    addSpots(sequence, spots, radius / 1.41, level - 1)


def create(mediaWindow):
    spots = []
    sequence = MovingShapeSequence("Component tester")
    addSpots(sequence, spots, 100, 4)
    sequence.setAgenda(
        [
            Stimulus.Blank(duration_s=1),
            StartMeasurement(),
            Stimulus.Blank(duration_s=1),
            Stimulus.Generic(duration_s=20, shape=Shape.MultiSpot(spots=spots)),
            Stimulus.Blank(duration_s=1),
            EndMeasurement(),
            Stimulus.Blank(duration_s=1),
        ]
    )
    return sequence
