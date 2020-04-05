import Gears as gears
from .. import * 
import traceback
import inspect
from .Base import *

class Generic(Base) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(
            self,
            *,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Generic',
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            = 1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            = 0 ,
            forward         : 'Component for forward rendering. (Forward.*)'
                            = Forward.Nop() ,
            passes         : 'List of passes'
                            = [],
            signal          : 'Component for signals. (Signal.*)'
                            = Signal.FlatWithTicks(),
            toneMapping     : 'Component for tone mapping (Tone.*)'
                            = Tone.UiConfigured(),
            gamma           : 'Component for gamma compensation. (Gamma.*)'
                            = Gamma.Linear()        ,
            temporalFilter  : 'Component for temporal filtering. (Temporal.*)'
                            = Temporal.Nop()        ,
            spatialFilter   : 'Component for spatial filtering. (Spatial.*)'
                            = Spatial.Nop()         ,
            prng            : 'Component for pseudo-random number generation. (Prng.*)'
                            = Prng.Nop(),
            particleSystem  : 'Component for particle system simulation. (ParticleSystem.*)'
                            = ParticleSystem.Nop()
            ):
        self.name                =      name
        self.duration            =      duration
        sequence = self.getSequence()
        if(duration_s != 0):
            self.duration = int(duration_s // sequence.getFrameInterval_s() + 1)
        
        forward      .apply(self)
        signal       .apply(self)
        toneMapping  .apply(self)
        gamma        .apply(self)
        temporalFilter.apply(self)
        prng         .apply(self)
        particleSystem.apply(self)
        spatialFilter.apply(self)

        for Pass in passes:
            self.addPass(Pass)


