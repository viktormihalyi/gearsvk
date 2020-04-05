import Gears as gears
from .. import * 
from .SingleShape import *

class ShiftingBarcode(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'ShiftingBarcode',
            randomSeed      : 'Initial state of the pseudo-random generator.'
                            = 3773623027,
            randomGridSize  : 'Dimensions of the 2D array of randoms generated. Zero means viewport resolution.'
                            = (0, 1),
            color1          : 'Brightest color possible. (r,g,b) triplet or color name.'
                            = 'white',
            color2          : 'Darkest color possible. (r,g,b) triplet or color name.'
                            = 'black',
            spatialFilter   : 'Spatial filter component. (Spatial.*)'
                            = Spatial.Nop(),
            step            : 'The shifting applied to the random array as an x,y pair. (e.g. (-1, 0) means shift left. )'
                            = (-1, 0),
            initialValue    : 'The starting value as an (r,g,b) triplet, or "random" to randomize array in the first frame of the stimulus, "keep" to keep values from previous stimulus.'
                            = 'keep',
            shape           : 'Masking shape. (Pif.*)'
                            = Pif.RandomGrid()
            ):

        sequence = self.getSequence().getPythonObject()
        randomGridSize = (sequence.field_width_px if randomGridSize[0] == 0 else randomGridSize[0], sequence.field_height_px if randomGridSize[1] == 0 else randomGridSize[1])

        super().boot(name=name, duration=duration, duration_s=duration_s,
                shape = shape,
                pattern = Pif.Solid(
                    color=color1,
                    ),
                background = Pif.Solid(
                    color=color2,
                    ),
                prng =  Prng.CellShift(
                    randomSeed = randomSeed,
                    randomGridSize = randomGridSize,
                    step = step,
                    initialValue = initialValue,
                    ),
                spatialFilter = spatialFilter,
                )


