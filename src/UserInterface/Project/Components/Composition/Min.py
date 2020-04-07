import Gears as gears
from .. import * 
from ..Pif.Base import *

class Min(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            pif1     : 'First operand. (Pif.*)'
                        =   Pif.Solid( color = 'white' ),
            pif2     : 'Second operand. (Pif.*)'
                        =   Pif.Solid( color = 'white' )
            ) :
        stimulus = spass.getStimulus()
        pif1.apply(spass, functionName + '_op1')
        pif2.apply(spass, functionName + '_op2')

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<pattern>@ (vec2 x, float time){
                    return min( @<pattern>@_op1(x), @<pattern>@_op2(x) ); 
                }
        ''').format( pattern=functionName )  ) 

