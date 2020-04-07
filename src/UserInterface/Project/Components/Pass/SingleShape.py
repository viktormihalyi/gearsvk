import Gears as gears
from .. import * 
import traceback
import inspect
from .Generic import *

class SingleShape(Generic) : 

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
            rasterizingMode : 'The type of geometry to be rasterized for the spass. (fullscreen/bezier/triangles/quads)'
                            = 'fullscreen',
            polygonMask     : 'Data defining the geometry to be rasterized.'
                            = [{'x':0, 'y':0}, {'x':0, 'y':1}, {'x':1, 'y':0}],
            polygonMotion   : 'Motion component for polygon animation. Defaults to "shapeMotion" if none given. (Motion.*)'
                            = None
            ):
        if polygonMotion == None:
            polygonMotion = shapeMotion
        if not 'displayName' in modulation.args :
            modulation.args['displayName'] = 'Intensity'

        rootPif = ((shape << warp << shapeMotion ) ** (pattern << warp << patternMotion, background << warp << backgroundMotion)) << modulation

        super().boot(
            name=name,
            duration=duration,
            duration_s=duration_s,
            pif = rootPif,
            rasterizingMode=rasterizingMode,
            polygonMask=polygonMask,
            polygonMotion = polygonMotion,
        )


