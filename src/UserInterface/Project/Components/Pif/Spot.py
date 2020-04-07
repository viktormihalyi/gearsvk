import Gears as gears
from .. import * 
from .Base import *

class Spot(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
             *,
            radius          : 'Spot radius [um]., or Interactive.*'
                            = 200,
            innerRadius     : 'Annulus inner radius [um] (negative for solid disc), or Interactive.*'
                            = -1000,
            filterRadius_um : 'Antialiasing filter size [um] , or Interactive.* (shape blur).'
                            = 2.0
            ) :
        stimulus = spass.getStimulus()
        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                spotRadius      =   radius,
                spotInnerRadius =   innerRadius,
                filterRadius    =   filterRadius_um,
                )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    float diff = length(x);
                    float inOrOut = (1-smoothstep( -`filterRadius, +`filterRadius, diff - `spotRadius )) 
                        * (1-smoothstep( -`filterRadius, +`filterRadius, `spotInnerRadius - diff ));
                    return vec3(inOrOut, inOrOut, inOrOut);
             }
        ''').format( X=functionName )  ) 

