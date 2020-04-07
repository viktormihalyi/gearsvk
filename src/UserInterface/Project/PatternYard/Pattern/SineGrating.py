import Gears as gears
from .. import * 

class SineGrating(Component) : 

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
            wavelength          : 'Distance between wave crests [um].'
                                = 100,
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
        patch.setShaderVariable( name = 'sineGratingWavelength', value=wavelength )

        direction = processDirection(direction, self.tb)
        experiment = stimulus.getExperiment()
        s = math.sin(direction)
        c = math.cos(direction)
        patch.setShaderVector( name='span', x=c, y=s )
        
        patch.setShaderFunction( name = patternFunctionName, src = '''
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
                    float t = dot(x, span);
                    return mix(background, mix(color1, color2, 0.5-0.5*cos(t/sineGratingWavelength * 6.283185307179586476925286766559) ), shapeMask); }}
        '''.format( pattern=patternFunctionName )  ) 

