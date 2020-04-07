import Gears as gears
from .. import * 
from .Base import *

class Solid(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color               : 'Solid pattern color, or Interactive.*'
                                = 'white'
            ) :
        
        color = processColor(color, self.tb)
   
       
        if not isGrey(color):
            spass.enableColorMode()

        stimulus = spass.getStimulus()
        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                color       =        color,
                )

        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    return @<X>@_color; }
        ''').format( X=functionName )  ) 

