import Gears as gears
from .. import * 
from .Base import *

class Gaussian(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
             *,
            variance        : 'Gaussian variance [um], or Interactive.*'
                            = 200
            ) :
        stimulus = spass.getStimulus()
        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                variance = variance
                )
        #spass.setShaderVariable( name = functionName+'_variance',    value = variance )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    float diff = length(x);
                    float inOrOut = exp(-diff / 2 / `variance );
                    return vec3(inOrOut, inOrOut, inOrOut);
             }
        ''').format( X=functionName )  ) 

