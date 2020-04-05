import Gears as gears
from .. import * 
from .Base import *

class ForwardRendered(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            background  : 'Color outside of shape.'
                        = 'black'
            ) :


        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                #ifndef GEARS_FORWARD_FRAME_RESOURCES
                #define GEARS_FORWARD_FRAME_RESOURCES
                uniform sampler2D forwardImage;
                #endif
                vec3 @<X>@ (vec2 x, float time){ 
                    return texture(forwardImage, (x + patternSizeOnRetina * 0.5 )/patternSizeOnRetina * vec2(1, -1) ).xyz;
                }
         ''').format( X=functionName )  ) 

