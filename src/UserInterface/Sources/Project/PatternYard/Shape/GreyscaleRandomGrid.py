import GearsModule as gears
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
                ivec2 iv = ivec2 ((x + patternSizeOnRetina * 0.5) / cellSize);
                float v = float(randoms[randoms_layerIndex][iv.y][iv.x].x) / 0xffffffff;
                return vec3(v, v, v);
            }
        '''  )

