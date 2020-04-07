import Gears as gears
from .. import * 
from .Base import *

class Polarize(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            radius  : 'The radius of the warped field [um].'
                    = 300,
            innerRadius  : 'The inner radius of the warped field [um].'
                    = 0,
            startAngle  : 'The angle corresponding to the left edge of the field [radian].'
                    = -math.pi,
            endAngle  : 'The angle corresponding to the right edge of the field [radian].'
                    = math.pi
            ) :

        sequence = spass.getSequence()

        spass.setShaderVector( name = functionName+'_angle', x= sequence.field_width_um / (endAngle - startAngle), y = startAngle * 0.5 + endAngle * 0.5 )
        spass.setShaderVector( name = functionName+'_radius', x= sequence.field_height_um / (radius - innerRadius), y = innerRadius * 0.5 + radius * 0.5  )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){return vec2( (atan(x.y, x.x) - `angle.y) * `angle.x  , (length(x) - `radius.y) * `radius.x) ;}
        ''').format( X=functionName )  ) 
