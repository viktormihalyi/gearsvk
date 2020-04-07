import Gears as gears
from .. import * 
from .Base import *

class Solid(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color               : 'Solid pattern color.'
                                = 'white'
            ) :
        color = processColor(color, self.tb)
   
        stimulus = spass.getStimulus()
        if max(color) - min(color) > 0.03:
            stimulus.enableColorMode()

        spass.setShaderColor( name = functionName + '_color', red = color[0], green=color[1], blue=color[2] )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    return @<X>@_color; }
        ''').format( X=functionName )  ) 

