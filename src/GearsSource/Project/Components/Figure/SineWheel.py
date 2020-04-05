import Gears as gears
from .. import * 
from .Base import *

class SineWheel(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color1              : '"bright" pattern color.'
                                = 'white',
            color2              : '"dark" pattern color.'
                                = 'black',
            direction           : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')."
                                = 'east',
            wavelength_radians  : 'Angle between wave crests [radians].'
                                = 6.283185307179586476925286766559 / 17
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)

        stimulus = spass.getStimulus()
        if max(color1) - min(color1) > 0.03 or max(color2) - min(color2) > 0.03:
            stimulus.enableColorMode()

        spass.setShaderColor( name = functionName+'_color1', red = color1[0], green=color1[1], blue=color1[2] )
        spass.setShaderColor( name = functionName+'_color2', red = color2[0], green=color2[1], blue=color2[2] )
        spass.setShaderVariable( name = functionName+'_sineWheelWavelength', value=wavelength_radians )

        direction = processDirection(direction, self.tb)
        sequence = stimulus.getSequence()
        s = math.sin(direction)
        c = math.cos(direction)
        spass.setShaderVector( name=functionName+'_span', x=c, y=s )
        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    float u = dot(normalize(x), `span);
                    float v = cross(vec3(normalize(x),0), vec3(`span, 0)).z;
                    float t = atan(u, v);
                    return mix(`color1, `color2, 0.5-0.5*cos(t/`sineWheelWavelength * 6.283185307179586476925286766559)); }
        ''').format( X=functionName )  ) 

