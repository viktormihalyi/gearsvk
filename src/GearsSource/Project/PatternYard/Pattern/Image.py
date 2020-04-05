import Gears as gears
from .. import * 

class Image(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            imagePath           : 'Image file name with path.'
                                = None, 
            background          : 'Color outside of shape.'
                                = 'black',
            textureName         : 'Shader variable name (used to avoid naming conflicts when multiple image patterns are used).'
                                = 'image1',
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                ='pattern',
            grayScale           : 'A True value indicates the image can be handled as grayscale. This allows saving bandwidth and improved performance.'
                                = False
            ) :
        if imagePath == None:
            raise TypeError('imagePath is a required keyword argument for Pattern.Image.')

        stimulus = patch.getStimulus()
        if not grayScale or max(background) - min(background) > 0.03 :
            stimulus.enableColorMode()

        background = processColor(background, self.tb)
        patch.setShaderColor( name = 'background', red = background[0], green=background[1], blue=background[2] )
        patch.setShaderImage(textureName, imagePath)
        patch.setShaderFunction( name = patternFunctionName, src = '''
                uniform sampler2D {image1};
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
                    vec3 color = texture({image1}, (x + patternSizeOnRetina * 0.5 )/patternSizeOnRetina ).xyz;
                    return mix(background, color, shapeMask);
                    }}
        '''.format( image1 = textureName, pattern=patternFunctionName )  )

