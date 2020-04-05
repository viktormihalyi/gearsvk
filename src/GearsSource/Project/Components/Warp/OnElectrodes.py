import Gears as gears
from .. import * 
from .Base import *

class OnElectrodes(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            period  : 'Distance between two neighboring electrodes as an (x,y) pair [(um,um)]. 0 means sequence.electrodeDistance_um is used.'
                    = (0, 0)
            ) :

        sequence = spass.getSequence().getPythonObject()

        horizontalPeriod = period[0]
        verticalPeriod = period[1]
        if horizontalPeriod == 0 :
            horizontalPeriod = sequence.electrodeDistance_um[0]
        if verticalPeriod == 0 :
            verticalPeriod = sequence.electrodeDistance_um[1]
        period = (horizontalPeriod, verticalPeriod)

        spass.setShaderVector( name = functionName+'_period', x= horizontalPeriod, y = verticalPeriod )
        spass.setShaderVector( name = functionName+'_offset', x= sequence.electrodeOffset_um[0], y = sequence.electrodeOffset_um[1] )
        spass.setShaderVector( name = functionName+'_zone1TopLeft', x = sequence.electrodeZone1[0], y = sequence.electrodeZone1[1] )
        spass.setShaderVector( name = functionName+'_zone1BottomRight', x = sequence.electrodeZone1[2], y = sequence.electrodeZone1[3] )
        spass.setShaderVector( name = functionName+'_zone2TopLeft', x = sequence.electrodeZone2[0], y = sequence.electrodeZone2[1] )
        spass.setShaderVector( name = functionName+'_zone2BottomRight', x = sequence.electrodeZone2[2], y = sequence.electrodeZone2[3] )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){
                    vec2 p = x - @<X>@_offset + @<X>@_period * 0.5;
                    vec2 ip = floor(p / @<X>@_period);
                    if( ip.x >= @<X>@_zone1TopLeft.x && ip.y >= @<X>@_zone1TopLeft.y &&
                        ip.x <= @<X>@_zone1BottomRight.x && ip.y <= @<X>@_zone1BottomRight.y
                        ||
                        ip.x >= @<X>@_zone2TopLeft.x && ip.y >= @<X>@_zone2TopLeft.y &&
                        ip.x <= @<X>@_zone2BottomRight.x && ip.y <= @<X>@_zone2BottomRight.y 
                    )
                        return mod(p, @<X>@_period) - @<X>@_period * 0.5;
                    else
                        return vec2(1000000, 1000000);
                    }
                ''').format( X=functionName )  ) 
