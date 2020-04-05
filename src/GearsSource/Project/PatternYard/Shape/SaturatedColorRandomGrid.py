import Gears as gears
from .. import * 

class SaturatedColorRandomGrid(Component) : 

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
                uvec3 v = texelFetch(randoms, ivec2( (x + patternSizeOnRetina*0.5) / cellSize), 0).xyz;
                return vec3(
                    (v.x >> 31u == 0u)?0.0:1.0,
                    (v.y >> 31u == 0u)?0.0:1.0,
                    (v.z >> 31u == 0u)?0.0:1.0);
                    ;
            }
        '''  )

