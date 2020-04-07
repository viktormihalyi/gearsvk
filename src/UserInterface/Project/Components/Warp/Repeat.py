import Gears as gears
from .. import * 
from .Base import *

class Repeat(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            period  : 'Horizontal and vertical periods with which the image is repeated as an (x,y) pair [(um,um)].'
                    = (0, 0)
            ) :

        sequence = spass.getSequence()

        horizontalPeriod = period[0]
        verticalPeriod = period[1]
        if horizontalPeriod == 0 :
            horizontalPeriod = sequence.field_width_um
        if verticalPeriod == 0 :
            verticalPeriod = sequence.field_height_um
        period = (horizontalPeriod, verticalPeriod)

        spass.setShaderVector( name = functionName+'_period', x= horizontalPeriod, y = verticalPeriod )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){return mod(x + `period * 0.5, `period) - `period * 0.5;}
        ''').format( X=functionName )  ) 
