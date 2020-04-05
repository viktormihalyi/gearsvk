import Gears as gears
from .. import * 
import traceback
import inspect
from .SinglePass import *

class SingleShape(SinglePass) : 

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
            shape           : 'Component for shape. (Shape.*)'
                            = Pif.Solid()     ,
            shapeMotion     : 'Motion component for shape animation. (Motion.*)'
                            = Motion.Linear()       ,
            pattern         : 'Component for pattern. (Pif.*)'
                            = Pif.Solid()       ,
            patternMotion   : 'Motion component for pattern animation. (Motion.*)'
                            = Motion.Linear()       ,
            background      : 'Component for pattern. (Pif.*)'
                            = Pif.Solid(color = 'black')       ,
            backgroundMotion: 'Motion component for background animation. (Motion.*)'
                            = Motion.Linear()       ,
            modulation      : 'Component for temporal modulation. (Modulation.*)'
                            = Modulation.Linear()   ,
            warp            : 'Component for image distortion. (Warp.*)'
                            = Warp.Nop()            ,
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
                            = ParticleSystem.Nop(),
            rasterizingMode : 'The type of geometry to be rasterized for the spass. (fullscreen/bezier/triangles/quads)'
                            = 'fullscreen',
            polygonMask     : 'Data defining the geometry to be rasterized.'
                            = [{'x':0, 'y':0}, {'x':0, 'y':1}, {'x':1, 'y':0}],
            polygonMotion   : 'Motion component for polygon animation. Defaults to "shapeMotion" if none given. (Motion.*)'
                            = None
            ):
        if polygonMotion == None:
            polygonMotion = shapeMotion
        super().boot(name=name, duration=duration, duration_s=duration_s,
                    forward = forward,
                    signal = signal,
                    gamma = gamma,
                    toneMapping = toneMapping,
                    temporalFilter = temporalFilter,
                    spatialFilter = spatialFilter,
                    prng = prng,
                    particleSystem = particleSystem,
                    spass = Pass.SingleShape(
                        shape            = shape          ,
                        shapeMotion      = shapeMotion    ,
                        pattern          = pattern        ,
                        patternMotion    = patternMotion  ,
                        background       = background     ,
                        backgroundMotion = backgroundMotion  ,
                        modulation       = modulation     ,
                        warp             = warp           ,
                        rasterizingMode  = rasterizingMode      ,
                        polygonMask      = polygonMask      ,
                        polygonMotion    = polygonMotion    ,
                    ),
            )

