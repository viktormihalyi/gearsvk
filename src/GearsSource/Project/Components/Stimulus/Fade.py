import Gears as gears
from .. import * 
import traceback
from .SinglePass import *

class Fade(SinglePass) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(self,
             *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Fade',
            forward         : 'Component for forward rendering. (Forward.*)'
                            = Forward.Nop()         ,
            shape           : 'Shape component.'
                            = Pif.Solid()     ,
            shapeMotion     : 'Shape motion component.'
                            = Motion.Linear()       ,
            pattern1        : 'First pattern component participating in the fading.'
                            = Pif.Solid( )       ,
            pattern2        : 'Second pattern component participating in the fading.'
                            = Pif.Solid( )       ,
            patternMotion   : 'Pattern motion component.'
                            = Motion.Linear()       ,
            modulation      : 'Modulation component.'
                            = Modulation.Linear()   ,
            warp            : 'Warp component.'
                            = Warp.Nop()            ,
            signal          : 'Signal component.'
                            = Signal.FlatWithTicks(),
            gamma           : 'Gamma component.'
                            = Gamma.Linear()        ,
            temporalFilter  : 'Temporal filter component.'
                            = Temporal.Nop()        ,
            spatialFilter   : 'Spatial filter component.'
                            = Spatial.Nop()         ,
            prng            : 'Pseudo-random number generator component.'
                            = Prng.Nop()
            ):
        super().boot(name=name, duration=duration, duration_s=duration_s,
                    forward = forward,
                    signal = signal,
                    gamma = gamma,
                    temporalFilter = temporalFilter,
                    spatialFilter = spatialFilter,
                    prng = prng,
                    spass = Pass.Fade(
                        shape            = shape          ,
                        shapeMotion      = shapeMotion    ,
                        pattern2         = pattern2       ,
                        pattern1         = pattern1       ,
                        patternMotion    = patternMotion  ,
                        modulation       = modulation     ,
                        warp             = warp           ,
                    ),
            )
