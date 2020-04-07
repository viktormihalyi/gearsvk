import Gears as gears
from .. import * 
from .Base import *

class Video(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                ='pattern',
            videoFileName       : 'The name of the video file to play (with path).'
                                = None,
            videoContrast       : '0 - no contrast, 1 - original contrast, or Interactive.*'
                                = 1,
            videoBrightness     : 'Additional brightness. 0 - original brightness, or Interactive.*'
                                = 0,
            greyscale           : 'If True, video is rendered in greyscale.'
                                = False
            ) :
        stimulus = spass.getStimulus()
        sequence = stimulus.getSequence()
        spass.setVideo(videoFileName)
        if not greyscale :
            spass.enableColorMode()

        self.registerInteractiveControls(
                spass, stimulus,
                '',
                videoContrast = videoContrast,
                videoBrightness = videoBrightness,
                )

        #spass.setShaderVariable( name = 'videoContrast', value = videoContrast)
        #spass.setShaderVariable( name = 'videoBrightness', value = videoBrightness)
        spass.setShaderVector( name = 'videoSize_um', x=sequence.field_width_um, y=sequence.field_height_um)
        spass.setShaderVector( name = 'videoPosition_um', x=0, y=0)
        spass.setShaderVector( name = 'videoClipFactorY', x=1, y=-1)
        spass.setShaderVector( name = 'videoClipFactorUV', x=1, y=-1)

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                #ifndef GEARS_VIDEO_FRAME_RESOURCES
                #define GEARS_VIDEO_FRAME_RESOURCES
                uniform sampler2D videoFrameY;
                uniform sampler2D videoFrameU;
                uniform sampler2D videoFrameV;
                #endif
                vec3 @<X>@ (vec2 x, float time){ 
                    vec3 yuv;
                    vec2 uv = (x - videoPosition_um - videoSize_um*0.5) / videoSize_um;
                    yuv.x = (texture2D(videoFrameY, fract(uv) * videoClipFactorY).x + videoBrightness);
                    yuv.y = texture2D(videoFrameU, fract(uv) * videoClipFactorUV).x;
                    yuv.z = texture2D(videoFrameV, fract(uv) * videoClipFactorUV).x;
                    
                    vec3 rgb;
                    rgb.x = 1.164 * (yuv.x - 16.0/256.0) + 1.596 * (yuv.z - 128.0/256.0);
                    rgb.y = 1.164 * (yuv.x - 16.0/256.0) - 0.813 * (yuv.z - 128.0/256.0) - 0.391 * (yuv.y - 128.0/256.0);
                    rgb.z = 1.164 * (yuv.x - 16.0/256.0)                                 + 2.018 * (yuv.y - 128.0/256.0);
                    
                    rgb.xyz = (rgb.xyz - vec3(0.5, 0.5, 0.5)) * videoContrast + vec3(0.5, 0.5, 0.5);

                    return rgb;
                    }
        ''').format( X=functionName )  ) 

