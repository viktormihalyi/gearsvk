import Gears as gears
from .. import * 
import traceback
import inspect
from .Generic import *

class Fade(Generic) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(
            self,
            *,
            name            : 'Pass name to display in sequence overview plot.'
                            = 'Generic',
            duration        : 'Pass time in frames. Defaults to stimulus duration. Superseded by duration_s is given.'
                            = 0,
            duration_s      : 'Pass time in seconds (takes precendece over duration given in frames).'
                            = 0,
            shape           : 'Component for shape. (Pif.*)'
                            = Pif.Solid()     ,
            shapeMotion     : 'Motion component for shape animation. (Motion.*)'
                            = Motion.Linear()       ,
            pattern1        : 'First pattern component participating in the fading. (Pif.*)'
                            = Pif.Solid()       ,
            pattern2        : 'Second pattern component participating in the fading. (Pif.*)'
                            = Pif.Solid()       ,
            patternMotion   : 'Motion component for pattern animation. (Motion.*)'
                            = Motion.Linear()       ,
            modulation      : 'Component for temporal modulation. (Modulation.*)'
                            = Modulation.Linear()   ,
            warp            : 'Component for image distortion. (Warp.*)'
                            = Warp.Nop()            
            ):

        if not 'displayName' in modulation.args :
            modulation.args['displayName'] = 'Fade factor'
        
        rootPif = ((shape << shapeMotion << modulation) ** (pattern1 << patternMotion, pattern2 << patternMotion)) << warp

        super().boot(name=name, duration=duration, duration_s=duration_s,
            pif = rootPif,
        )



