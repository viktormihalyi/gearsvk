import Gears as gears
from .. import * 
from .Base import *

class Mandelbrot(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color1              : '"bright" pattern color.'
                                = 'white',
            color2              : '"dark" pattern color.'
                                = 'black',
            direction           : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')"
                                = 'east'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)

        stimulus = spass.getStimulus()
        if not isGrey(color1) or not isGrey(color2):
            spass.enableColorMode()

        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                color1 = color1,
                color2 = color2,
                )

        #spass.setShaderColor( name = functionName+'_color1', red = color1[0], green=color1[1], blue=color1[2] )
        #spass.setShaderColor( name = functionName+'_color2', red = color2[0], green=color2[1], blue=color2[2] )

        direction = processDirection(direction, self.tb)
        sequence = spass.getSequence()
        s = math.sin(direction)
        c = math.cos(direction)
        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    vec2 c=x*0.002 /pow(2,time)  + vec2(0.375920, 0.193);
                    vec2 z=c;
                    int i=0;
                    for(; i<400; i++)
                    {
                        z = vec2( z.x*z.x - z.y*z.y + c.x, 2 * z.x*z.y + c.y );
                        if(dot(z,z) > 4)
                            break;
                    }
                    return mix(`color1, `color2, i * 0.01 ); 
                }
        ''').format( X=functionName )  ) 

