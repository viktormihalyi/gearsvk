import Gears as gears
from .. import * 
from .Base import *

class Nop(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName
            ) :
         
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){ return x; }
        ''').format( X=functionName )  ) 
