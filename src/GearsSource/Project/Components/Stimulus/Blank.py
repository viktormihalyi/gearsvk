import Gears as gears
from .. import * 
from .SingleShape import *

class Blank(SingleShape) :

    def __init__(self, **args):
        super().__init__( **args )

    def boot(self,
            *,
            duration    : 'Stimulus time in frames.' 
                        = 1,
            duration_s  : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                        = 0,
            name        : 'Stimulus name to display in sequence overview plot.'
                        = '.',
            color       : 'Base color as an rgb triplet or color name.'
                        = 'black',
            temporalFilter = Temporal.Nop()
            ):
        color = processColor(color, self.tb)
        intensity = max(color[0], color[1], color[2])
        if intensity > 0.001:
            color = (color[0] / intensity, color[1] / intensity, color[2] / intensity)
        #if name=='.' and isinstance(color, (int, float)) :
            name = '{intensity:.1f}'.format(intensity=intensity)
        
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     shape =   Pif.Solid( color=color),
                     signal     =   Signal.NoTicks(),
                     modulation = Modulation.Linear(intensity=intensity), 
                     temporalFilter = temporalFilter,
                     )

