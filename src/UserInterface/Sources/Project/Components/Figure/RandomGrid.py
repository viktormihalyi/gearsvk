import GearsModule as gears
from .. import *
from .Base import * 

class RandomGrid(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            ) :
        #randoms and cellSize are set from the C++ DLL
        spass.setShaderFunction( name = functionName, src = self.glslEsc( gears.GetGLSLResourcesForRandoms(spass.getStimulus()) + '''
            vec3 @<X>@ (vec2 x, float time){
                ivec2 iv = ivec2 ((x + randomGridSize * cellSize * 0.5) / cellSize);
                if(randoms[randoms_layerIndex][iv.y][iv.x].x >> 31u == 0u)
        		    return vec3(0, 0, 0);
	            else
		            return vec3(1, 1, 1);
            }
        ''').format( X=functionName )  ) 

