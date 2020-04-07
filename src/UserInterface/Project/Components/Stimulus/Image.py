import Gears as gears
from .. import * 
from .SingleShape import *

class Image(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Image',            
            imagePath       : 'Image file name with path.'
                            = None, 
            startPosition   : 'Initial position as an x,y pair [(um,um)].'
                            = (0, 0),
            velocity        : 'Motion velocity vector as an x,y pair [(um/s,um/s)].'
                            = (0, 0),
            temporalFilter  : 'Component for temporal filtering. (Temporal.*)'
                            = Temporal.Nop(),
            spatialFilter   : 'Spatial filter component. (Spatial.*)'
                            = Spatial.Nop()
            ):
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     pattern = Pif.Image( 
                            imagePath = imagePath
                            ),
                     patternMotion = Motion.Linear(
                            startPosition = startPosition,
                            velocity = velocity,
                            ),
                     warp = Warp.Repeat(),
                     temporalFilter = temporalFilter,
                     spatialFilter = spatialFilter,
                     )