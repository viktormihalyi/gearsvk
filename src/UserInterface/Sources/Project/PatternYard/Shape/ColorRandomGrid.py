import GearsModule as gears
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
                ivec2 iv = ivec2( (x + patternSizeOnRetina*0.5) / cellSize), 0);
                vec3 v = vec3(randoms[randoms_layerIndex][iv.y][iv.x].xyz) / 0xffffffff;
                return v;
            }
        '''  )

