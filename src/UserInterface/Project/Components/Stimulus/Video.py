import Gears as gears
from .. import * 
from .SingleShape import *

class Video(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Video',            
            videoPath       : 'Video file name with path.'
                            = None, 
            greyscale       : 'If True, video is rendered in greyscale.'
                            = False,
            startPosition   : 'Initial position as an x,y pair [(um,um)].'
                            = (0, 0),
            velocity        : 'Motion velocity vector as an x,y pair [(um/s,um/s)].'
                            = (0, 0),
            spatialFilter   : 'Spatial filter component. (Spatial.*)'
                            = Spatial.Nop(),
            temporalFilter  : 'Temporal filter component. (Temporal.*)'
                            = Temporal.Nop(),
            toneMapping     : 'Tone mapping component (Tone.*)'
                            = Tone.UiConfigured(),
            videoContrast   : '0 - no contrast, 1 - original contrast, or Interactive.*'
                            = None,
            videoBrightness : 'Additional brightness. 0 - original brightness, or Interactive.*'
                            = None
            ):
        if videoContrast == None:
            videoContrast = Interactive.MouseWheel(key = 'C', label = 'Contrast', initialValue = 1)
        if videoBrightness == None:
            videoBrightness = Interactive.MouseWheel(key = 'B', label = 'Brightness', minimum=-1, initialValue =0)

        super().boot(name=name, duration=duration, duration_s=duration_s,
                     pattern = Pif.Video( 
                            videoFileName = videoPath,
                            greyscale = greyscale,
                            videoContrast = videoContrast,
                            videoBrightness = videoBrightness,
                            ),
                     patternMotion = Motion.Linear(
                            startPosition = startPosition,
                            velocity = velocity,
                            ),
                     warp = Warp.Clamp(),
                     spatialFilter = spatialFilter,
                     temporalFilter = temporalFilter,
                     toneMapping = toneMapping,
                     )