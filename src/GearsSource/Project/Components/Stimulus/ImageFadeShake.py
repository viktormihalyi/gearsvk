import Gears as gears
from .. import * 
from .Fade import *

class ImageFadeShake(Fade) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'ImageFade',            
            imagePath1      : 'First image file name with path.'
                            = None, 
            imagePath2      : 'Second image file name with path.'
                            = None, 
            startPosition   : 'Initial position as an x,y pair [(um,um)].'
                            = (0, 0),
            velocity        : 'Motion velocity vector as an x,y pair [(um/s,um/s)].'
                            = (0, 0),
            weightingSlope  : 'Slope of linear function for weighting the two images [1/s] "hold" means 0, with weight 1 unless otherwise specified. "down" means weight goes from 1 to 0 in duration. "up" means 0 to 1.'
                            = 'hold',
            weighting       : 'Initial weight (in [0,1], or "unspecified" to find value using intensitySlope)'
                            = 'unspecified',
            boundingRadius  : 'Perturbation amplitude [um].'
                            = 100
            ):
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     pattern1 = Pif.Image( 
                            imagePath = imagePath1,
                            textureName = 'image1',
                            ),
                     pattern2 = Pif.Image( 
                            imagePath = imagePath2,
                            textureName = 'image2',
                            ),
                     modulation = Modulation.Linear(
                            intensity = weighting,
                            intensitySlope = weightingSlope,
                            ),
                     patternMotion = Motion.Shake(
                            startPosition = startPosition,
                            boundingRadius = boundingRadius,
                            ),
                     warp = Warp.Repeat(),
                     )