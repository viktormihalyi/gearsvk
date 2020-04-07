import Gears as gears
from .. import * 
from ..Pif.Base import *

class Add(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            shape               : 'Shape pif. (Pif.*)'
                                = Pif.Spot(),
            pif1                 : 'First operand. (Pif.*)'
                                = Pif.Solid( color = 'white' ),
            pif2                 : 'Second operand. (Pif.*)'
                                = Pif.Solid( color = 'white' )
            ) :
        stimulus = spass.getStimulus()
        pif1.apply(spass, functionName + '_op1')
        pif2.apply(spass, functionName + '_op2')

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<pif>@ (vec2 x, float time){
                    return  @<pif>@_op1(x, time) + @<pif>@_op2(x, time); 
                }
        ''').format( pif=functionName )  ) 

