import GearsModule as gears
from .. import * 
from .Base import *

class SaturatedColorRandomGrid(Base) : 

    def applyWithArgs(self,
            spass,
            functionName,) :
        #randoms and cellSize are set from the C++ DLL
        stimulus = spass.getStimulus()
        stimulus.enableColorMode()
        spass.setShaderFunction(name = functionName, src = self.glslEsc(gears.GetGLSLResourcesForRandoms() + '''
            vec3 @<X>@ (vec2 x, float time){ 
                ivec2 iv = ivec2 ((x + randomGridSize * cellSize * 0.5) / cellSize);
                uvec3 v = randoms[randoms_layerIndex][iv.y][iv.x].xyz;
                return vec3(
                    (v.x >> 31u == 0u)?0.0:1.0,
                    (v.y >> 31u == 0u)?0.0:1.0,
                    (v.z >> 31u == 0u)?0.0:1.0);
                    ;
            }
        ''').format(X=functionName)) 

