import Gears as gears
from .. import * 
from .Base import *

class PinchWhirl(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            radius  : 'The radius of the distortion circle [um].'
                    = 600,
            pinch  : 'The pinch factor.'
                    = -0.75,
            whirl  : 'The whirl factor.'
                    = 4
            ) :

        sequence = spass.getSequence()

        spass.setShaderVariable( name = functionName+'_radius', value= radius  )
        spass.setShaderVariable( name = functionName+'_pinch', value= pinch  )
        spass.setShaderVariable( name = functionName+'_whirl', value= whirl  )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){
                    float d2 = dot(x,x);
                    if(d2 > `radius * `radius)
                        return x;
                        float dist = sqrt(d2) / `radius;
                        float factor = pow (sin (1.5707963267948966192313216916398 * dist), `pinch);
                        x *= factor;
                        //whirl
                        factor = 1 - dist;
                        float ang = `whirl * factor * factor;
                        float sina = sin (ang);
                        float cosa = cos (ang);

                        return vec2(
                            cosa * x.x - sina * x.y,
                            sina * x.x + cosa * x.y);
                    }
        ''').format( X=functionName )  ) 
