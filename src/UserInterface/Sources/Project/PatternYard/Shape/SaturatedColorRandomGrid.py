import GearsModule as gears
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
                ivec2 iv = ivec2 ((x + patternSizeOnRetina*0.5) / cellSize);
                uvec3 v = randoms[randoms_layerIndex][iv.y][iv.x] .xyz;
                return vec3(
                    (v.x >> 31u == 0u)?0.0:1.0,
                    (v.y >> 31u == 0u)?0.0:1.0,
                    (v.z >> 31u == 0u)?0.0:1.0);
                    ;
            }
        '''  )

