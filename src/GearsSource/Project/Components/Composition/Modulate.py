import Gears as gears
from .. import * 
from ..Pif.Base import *

class Modulate(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            pif      : 'Pif to warp. (Pif.*)'
                        =  Pif.Solid( color = 'white' ),
            modulation  : 'Motion component. (Motion.*)'
                        =  Modulation.Linear( )
            ) :
        stimulus = spass.getStimulus()
        pif.apply(spass,     functionName + '_modulated')
        modulation.apply(spass, functionName + '_modulator')

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){
                    return `modulated(  x,  time) * `modulator(time); 
                }
        ''').format( X=functionName )  ) 
