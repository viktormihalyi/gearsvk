import Gears as gears
from .. import * 
from .Base import *

class Hammer(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            ) :

        sequence = spass.getSequence()

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 g){
                    float x = g.x * 0.0022;
                    float y = g.y * 0.0022;
                    float z = sqrt(1 - (x * 0.25) * (x * 0.25) - (y * 0.5) * (y * 0.5) );
                    return vec2( 2 * atan( z * x / (2 * (2 *z*z -1))), asin(y * z )  ) * 1000;
                    }
        ''').format( X=functionName )  ) 
