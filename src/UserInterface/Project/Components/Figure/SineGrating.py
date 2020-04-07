import Gears as gears
from .. import * 
from .Base import *

class SineGrating(Base) : 

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
            wavelength          : 'Distance between wave crests [um].'
                                = 100,
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                = 'pattern'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)

        stimulus = spass.getStimulus()
        if max(color1) - min(color1) > 0.03 or max(color2) - min(color2) > 0.03:
            stimulus.enableColorMode()

        spass.setShaderColor( name = functionName+'_color1', red = color1[0], green=color1[1], blue=color1[2] )
        spass.setShaderColor( name = functionName+'_color2', red = color2[0], green=color2[1], blue=color2[2] )
        spass.setShaderVariable( name = functionName+'_sineGratingWavelength', value=wavelength )

        direction = processDirection(direction, self.tb)
        sequence = stimulus.getSequence()
        s = math.sin(direction)
        c = math.cos(direction)
        spass.setShaderVector( name=functionName+'_span', x=c, y=s )
        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    float t = dot(x, `span);
                    return mix(`color1, `color2, 0.5-0.5*cos(t/`sineGratingWavelength * 6.283185307179586476925286766559)); }
        ''').format( X=functionName )  ) 

