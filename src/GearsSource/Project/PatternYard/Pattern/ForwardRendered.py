import Gears as gears
from .. import * 

class ForwardRendered(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            background  : 'Color outside of shape.'
                        = 'black',
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                                = 'pattern'
            ) :

        background = processColor(background, self.tb)
        patch.setShaderColor( name = 'background', red = background[0], green=background[1], blue=background[2] )

        patch.setShaderFunction( name = patternFunctionName, src = '''
                uniform sampler2D {image1};
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
                    vec3 color = texture({image1}, (x + patternSizeOnRetina * 0.5 )/patternSizeOnRetina ).xyz;
                    return mix(background, color, shapeMask);
                    }}
        '''.format( image1 = 'forwardImage', pattern=patternFunctionName )  )

