import GearsModule as gears
from .. import * 
from .Base import *

class GreyscaleRandomGrid(Base) : 

    def applyWithArgs(self,
            spass,
            functionName,) :
        #randoms and cellSize are set from the C++ DLL
        spass.setShaderFunction(name = functionName, src = self.glslEsc(gears.GetGLSLResourcesForRandoms() + '''
            vec3 @<X>@ (vec2 x, float time){ 
                ivec2 iv = ivec2 ((x + randomGridSize * cellSize * 0.5) / cellSize);
                float v = float(randoms[randoms_layerIndex][iv.y][iv.x].x) / float(0xffff) / float(0xffff);
                return vec3(v, v, v);

            }
        ''').format(X=functionName)) 

