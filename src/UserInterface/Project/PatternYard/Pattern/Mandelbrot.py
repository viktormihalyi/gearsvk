import Gears as gears
from .. import * 

class Mandelbrot(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            color1              : '"bright" pattern color.'
                                = 'white',
            color2              : '"dark" pattern color.'
                                = 'black',
            background          : 'Color outside of shape.'
                                = 'black',
            direction           : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')."
                                = 'east',
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                = 'pattern'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)
        background = processColor(background, self.tb)

        stimulus = patch.getStimulus()
        if max(color1) - min(color1) > 0.03 or max(color2) - min(color2) > 0.03 or max(background) - min(background) > 0.03:
            stimulus.enableColorMode()

        patch.setShaderColor( name = 'color1', red = color1[0], green=color1[1], blue=color1[2] )
        patch.setShaderColor( name = 'color2', red = color2[0], green=color2[1], blue=color2[2] )
        patch.setShaderColor( name = 'background', red = background[0], green=background[1], blue=background[2] )

        direction = processDirection(direction, self.tb)
        experiment = patch.getExperiment()
        s = math.sin(direction)
        c = math.cos(direction)
        patch.setShaderVector( name='span', x=c, y=s )
        
        patch.setShaderFunction( name = patternFunctionName, src = '''
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
                    vec2 c=x*0.002/pow(2,time) + vec2(0.375920, 0.193);
                    vec2 z=c;
                    int i=0;
                    for(; i<400; i++)
                    {{
                        z = vec2( z.x*z.x - z.y*z.y + c.x, 2 * z.x*z.y + c.y );
                        if(dot(z,z) > 4)
                            break;
                    }}
                    return mix(background, mix(color1, color2, i * 0.01 ), shapeMask); 
                }}
        '''.format( pattern=patternFunctionName )  ) 

