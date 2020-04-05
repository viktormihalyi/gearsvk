import Gears as gears
from .. import * 
from .Base import *
from .Linear import *

class Product(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            displayName     : 'The name that appears on the experience timeline plot. If None, this temporal modulation function is not displayed. '
                            = None,
            op0             : 'First op'
                            = Linear(),
            op1             : 'Second op'
                            = Linear()
            ) :

        if displayName:
            spass.registerTemporalFunction(functionName, displayName)

        op0.apply(spass, functionName + '_0')
        op1.apply(spass, functionName + '_1')


        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@(float time){ 
                    return (@<X>@_0(time) - 0.5) * (@<X>@_1(time) - 0.5) + 0.5;
                    }
        ''' ).format( X = functionName ) )