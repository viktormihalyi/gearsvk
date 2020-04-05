import Gears as gears
from .. import * 

class Video(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            background  : 'Color outside of shape.'
                        = 'black',
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                ='pattern',
            videoFileName       : 'The name of the video file to play (with path).'
                                = None,
            videoContrast       : '0 - no contrast, 1 - original contrast'
                                = 1,
            videoBrightness     : 'Additional brightness. 0 - original brightness.'
                                = 0,
            greyscale           : 'If True, video is rendered in greyscale.'
                                = False
            ) :
        stimulus = patch.getStimulus()
        experiment = stimulus.getExperiment()
        patch.setVideo(videoFileName)
        if not greyscale :
            stimulus.enableColorMode()

        background = processColor(background, self.tb)
        patch.setShaderColor( name = 'background', red = background[0], green=background[1], blue=background[2] )
        patch.setShaderVariable( name = 'videoContrast', value = videoContrast)
        patch.setShaderVariable( name = 'videoBrightness', value = videoBrightness)
        patch.setShaderVector( name = 'videoSize_um', x=experiment.field_width_um, y=experiment.field_height_um)
        patch.setShaderVector( name = 'videoPosition_um', x=0, y=0)
        patch.setShaderVector( name = 'videoClipFactorY', x=1, y=-1)
        patch.setShaderVector( name = 'videoClipFactorUV', x=1, y=-1)

        patch.setShaderFunction( name = patternFunctionName, src = '''
                uniform sampler2D videoFrameY;
                uniform sampler2D videoFrameU;
                uniform sampler2D videoFrameV;
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
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
                    //gl_FragColor.x = pow(gl_FragColor.x, gamma);
                    //gl_FragColor.y = pow(gl_FragColor.y, gamma);
                    //gl_FragColor.z = pow(gl_FragColor.z, gamma);

                    return mix(background, rgb, shapeMask);
                    }}
        '''.format( pattern=patternFunctionName )  )

