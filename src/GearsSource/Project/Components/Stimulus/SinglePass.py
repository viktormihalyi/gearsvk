import Gears as gears
from .. import * 
import traceback
import inspect
from .Generic import *

class SinglePass(Generic) : 

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
                            = Forward.Nop()         ,
            spass           : 'Component for stimulus rendering spass. (Pass.*)'
                            = None   ,
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
        if spass == None:
            raise SequenceError('No Pass given to SinglePass. "spass" is a required parameter.', traceback.extract_stack())
        super().boot(name=name, duration=duration, duration_s=duration_s,
            forward = forward,
            signal = signal,
            toneMapping = toneMapping,
            gamma = gamma,
            temporalFilter = temporalFilter,
            spatialFilter = spatialFilter,
            prng = prng,
            particleSystem = particleSystem,
            passes = [spass],
        )




