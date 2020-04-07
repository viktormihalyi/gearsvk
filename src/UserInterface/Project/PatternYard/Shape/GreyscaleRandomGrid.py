import Gears as gears
from .. import * 

class GreyscaleRandomGrid(Component) : 

    def applyWithArgs(
            self,
            patch,
            ) :
        patch.setShaderFunction( name = 'shape', src = '''
            uniform usampler2D randoms;
            uniform vec2 cellSize;
		    vec3 shape(vec2 x){ 
                float v = float(texelFetch(randoms, ivec2( (x + patternSizeOnRetina*0.5) / cellSize), 0).x) / 0xffffffff;
                return vec3(v, v, v);
            }
        '''  )

