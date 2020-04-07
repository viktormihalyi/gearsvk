import Gears as gears
from .. import * 
from .Base import *

class Spot(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
             *,
            radius          : 'Spot radius [um].'
                            = 200,
            innerRadius     : 'Annulus inner radius [um] (or negative for solid disc).'
                            = -1000,
            filterRadius_um : 'Antialiasing filter size [um] (shape blur).'
                            = 0.1
            ) :
        spass.setShaderVariable( name = functionName+'_spotRadius',    value = radius )
        spass.setShaderVariable( name = functionName+'_spotInnerRadius',    value = innerRadius )
        spass.setShaderVariable( name= functionName+'_filterRadius', value = filterRadius_um )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    float diff = length(x);
                    float inOrOut = (1-smoothstep( -`filterRadius, +`filterRadius, diff - `spotRadius )) 
                        * (1-smoothstep( -`filterRadius, +`filterRadius, `spotInnerRadius - diff ));
                    return vec3(inOrOut, inOrOut, inOrOut);
             }
        ''').format( X=functionName )  ) 

