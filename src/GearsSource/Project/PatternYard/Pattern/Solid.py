import Gears as gears
from .. import * 

class Solid(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            color               : 'Solid pattern color.'
                                = 'white',
            background          : 'Color outside of shape.'
                                = 'black',            
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                = 'pattern'
            ) :
        color = processColor(color, self.tb)
        background = processColor(background, self.tb)

        stimulus = patch.getStimulus()
        if max(color) - min(color) > 0.03 or max(background) - min(background) > 0.03:
            stimulus.enableColorMode()

        patch.setShaderColor( name = 'color', red = color[0], green=color[1], blue=color[2] )
        patch.setShaderColor( name = 'background', red = background[0], green=background[1], blue=background[2] )
        patch.setShaderFunction( name = patternFunctionName, src = '''
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
                    return mix(background, color, shapeMask); }}
        '''.format( pattern=patternFunctionName )  ) 

