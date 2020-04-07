import Gears as gears
from .. import * 

class ColorRandomGrid(Component) : 

    def applyWithArgs(
            self,
            patch,
            ) :
        stimulus = patch.getStimulus()
        stimulus.enableColorMode()
        patch.setShaderFunction( name = 'shape', src = '''
            uniform usampler2D randoms;
            uniform vec2 cellSize;
		    vec3 shape(vec2 x){ 
                vec3 v = vec3(texelFetch(randoms, ivec2( (x + patternSizeOnRetina*0.5) / cellSize), 0).xyz) / 0xffffffff;
                return v;
            }
        '''  )

