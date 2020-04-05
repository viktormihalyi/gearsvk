import Gears as gears
from .. import * 

class RandomGrid(Component) : 

    def applyWithArgs(
            self,
            patch,
            ) :
        #randoms and cellSize are set from the C++ DLL
        patch.setShaderFunction( name = 'shape', src = '''
            uniform usampler2D randoms;
            uniform vec2 cellSize;
		    vec3 shape(vec2 x){ 
                if(texelFetch(randoms, ivec2( (x + patternSizeOnRetina*0.5 ) / cellSize ) , 0).x >> 31u == 0u)
        		    return vec3(0, 0, 0);
	            else
		            return vec3(1, 1, 1);
            }
        '''  )

