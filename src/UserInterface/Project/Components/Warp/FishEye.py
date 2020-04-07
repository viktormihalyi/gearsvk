import Gears as gears
from .. import * 
from .Base import *

class FishEye(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            radius  : 'The radius of the inversion circle[um].'
                    = 300
            ) :

        sequence = spass.getSequence()

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){
                    float theta = atan(x.y, x.x);
                    float r = length(x);
                    r = pow(r, 0.7)*10;
                    return  vec2(cos(theta), sin(theta))*r;
                    }
        ''').format( X=functionName )  ) 
