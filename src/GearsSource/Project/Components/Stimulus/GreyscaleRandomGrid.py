import Gears as gears
from .. import * 
from .SingleShape import *

class GreyscaleRandomGrid(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            ='GreyscaleRandomGrid',
            randomSeed      : 'Initial state of the pseudo-random generator.'
                            = 3773623027,
            randomGridSize  : 'Dimensions of the 2D array of randoms generated. '
                            = (38, 38),
            color1          : 'Birghtest color possible. (r,g,b) triplet or color name.'
                            = 'white',
            color2          : 'Darkest color possible. (r,g,b) triplet or color name.'
                            = 'black',
            spatialFilter   : 'Spatial filter component. (Spatial.*)'
                            = Spatial.Nop()
            ):
        super().boot(name=name, duration=duration, duration_s=duration_s,
                shape = Pif.GreyscaleRandomGrid( 
                    ),
                pattern = Pif.Solid(
                    color=color1,
                    ),
                background = Pif.Solid(
                    color=color2,
                    ),
                prng =  Prng.XorShift128(
                    randomSeed = randomSeed,
                    randomGridSize = randomGridSize,
                    ),
                spatialFilter = spatialFilter,
                )


