import Gears as gears
from .. import * 
from .Base import *

class Clamp(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            minima  : 'Horizontal and vertical minima [(um,um)].'
                    = ('field', 'field'),
            maxima  : 'Horizontal and vertical maxima [(um,um)].'
                    = ('field', 'field')
            ) :

        sequence = spass.getSequence()

        hmin = minima[0]
        vmin = minima[1]
        if hmin == 'field' :
            hmin = -sequence.field_width_um * 0.5
        if vmin == 'field' :
            vmin = - sequence.field_height_um * 0.5
        minima = (hmin, vmin)

        hmax = maxima[0]
        vmax = maxima[1]
        if hmax == 'field' :
            hmax = sequence.field_width_um * 0.5
        if vmax == 'field' :
            vmax = sequence.field_height_um * 0.5
        maxima = (hmax, vmax)

        spass.setShaderVector( name = functionName+'_minima', x= minima[0], y = minima[1] )
        spass.setShaderVector( name = functionName+'_maxima', x= maxima[0], y = maxima[1] )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){return clamp(x, `minima, `maxima);}
        ''').format( X=functionName )  ) 
