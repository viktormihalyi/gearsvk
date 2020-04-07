import Gears as gears
from .. import * 
from .Base import *

class Orthographic(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            ) :

        sequence = spass.getSequence()

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 g){
                    float x = g.y* 0.001;
                    float y = g.x* 0.001;
                    
                    float rho = sqrt(x*x + y*y);
                    float c = asin(rho);
                    float phi1 = 1;
                    float la1 = 1;

                    return vec2( 
                        1000 * asin(cos(c)*sin(phi1) + y * sin(c) * cos(phi1) / rho),
                        1000 * (la1 + atan(x * sin(c), rho * cos(phi1) * cos(c) - y * sin(phi1) * sin(c)) )
                        );
                    }
        ''').format( X=functionName )  ) 
