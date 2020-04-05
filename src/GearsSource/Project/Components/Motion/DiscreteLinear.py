import Gears as gears
from .. import * 
from .Base import *

class DiscreteLinear(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            timestep        : 'The duration of a single position [s].'
                            = 1,
            jump            : 'Position change after every step [(um,um)].'
                            = (50, 0),
            startPosition   : 'Initial position as an x,y pair [(positionUnits,positionUnits)].'
                            = (0, 0),
            positionUnits   : "The unit of measurement for startPosition ('um', 'pixel', 'percent', or 'electrodeIndex')."
                            = 'um'
            ) :
        sequence = spass.getSequence().getPythonObject()
        if isinstance(startPosition[0], str) or positionUnits == 'electrodeIndex':
            code = ord(startPosition[0])
            if code >= ord('J') :
                code -= 1
            code -= ord('A')
            startPosition = (sequence.electrodeOffset_um[0] + code * sequence.electrodeDistance_um[0],
                            sequence.electrodeOffset_um[1] + startPosition[1] * sequence.electrodeDistance_um[1])
        elif positionUnits == 'percent' :
            startPosition = ( sequence.field_width_um * startPosition[0], sequence.field_height_um * startPosition[1] )
        elif positionUnits == 'pixel' :
            startPosition = ( sequence.field_width_um * startPosition[0] / sequence.field_width_px, sequence.field_height_um * startPosition[1] / sequence.field_height_px )
        spass.setShaderVector( name = functionName + '_startPosition', x = startPosition[0], y=startPosition[1] )
        spass.setShaderVector( name = functionName + '_jump', x = jump[0], y=jump[1] )
        spass.setShaderVariable( name = functionName + '_tstep', value = timestep )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            mat3x2 @<X>@ (float time){ return mat3x2(vec2(1, 0), vec2(0, 1), -`startPosition - `jump * floor(time / `tstep) ); }
            ''' ).format( X = functionName ) )


