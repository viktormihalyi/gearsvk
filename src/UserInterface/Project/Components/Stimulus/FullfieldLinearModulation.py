import Gears as gears
from .. import * 
from .SingleShape import *

class FullfieldLinearModulation(SingleShape) :

    def boot(self,
            *,  
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'FF linmod',
            intensitySlope  : "Intensity change per second (or 'up' - 0 to 1 in duration_s, 'down' - 1 to 0 in duration_s, or 'hold')"
                            = 'hold',
            **bargs         : Modulation.Linear
            ):
        if name == 'FF linmod' and type(intensitySlope) == str :
            name = intensitySlope
        super().boot(name=name, duration=duration, duration_s=duration_s,
                modulation = Modulation.Linear( intensitySlope=intensitySlope, **bargs ),
                )