import Gears as gears
from .. import * 
from .Base import *

class GeometricInversion(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            radius  : 'The radius of the inversion circle[um].'
                    = 300
            ) :

        sequence = spass.getSequence()

        spass.setShaderVariable( name = functionName+'_radius', value= radius  )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){return x * `radius * `radius / dot(x,x) ;}
        ''').format( X=functionName )  ) 
