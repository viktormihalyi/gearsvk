import Gears as gears
from .. import * 
from .SingleShape import *

class Fullfield(SingleShape) :

    def boot(self,
            *,  
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'fullfield',
            color           : 'Color as an (r,g,b) triplet or a color name.'
                            = 'white',
            **bargs         : Modulation.Linear ):
        super().boot(name=name, duration=duration, duration_s=duration_s,
                pattern = Pif.Solid(
                            color = color,
                            ),
                modulation = Modulation.Linear(**bargs),
                )