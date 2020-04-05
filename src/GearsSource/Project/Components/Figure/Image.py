import Gears as gears
from .. import * 
from .Base import *

class Image(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            imagePath           : 'Image file name with path.'
                                = None, 
            textureName         : 'Shader variable name (used to avoid naming conflicts when multiple image patterns are used).'
                                = 'image1',
            grayScale           : 'A True value indicates the image can be handled as grayscale. This allows saving bandwidth and improved performance.'
                                = False
            ) :
        if imagePath == None:
            raise TypeError('imagePath is a required keyword argument for Pattern.Image.')

        stimulus = spass.getStimulus()
        if not grayScale:
            stimulus.enableColorMode()

        spass.setShaderImage(functionName + '_' + textureName, imagePath)
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                uniform sampler2D @<image1>@ ;
                vec3 @<X>@(vec2 x, float time){
                    vec3 color = texture(@<image1>@, (x + patternSizeOnRetina * 0.5 )/patternSizeOnRetina ).xyz;
                    return color;
                    }
        ''').format( X=functionName, image1 = functionName + '_' + textureName )  ) 

