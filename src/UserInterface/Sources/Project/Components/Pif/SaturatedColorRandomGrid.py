import GearsModule as gears
from .. import * 
from .Base import *

class SaturatedColorRandomGrid(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            ) :
        #randoms and cellSize are set from the C++ DLL
        stimulus = spass.getStimulus()
        spass.enableColorMode()
        spass.setShaderFunction( name = functionName, src = self.glslEsc( gears.GetGLSLResourcesForRandoms () + '''
            vec3 @<X>@ (vec2 x, float time){ 
                uvec3 v = texelFetch(randoms, ivec2( (x + randomGridSize * cellSize*0.5) / cellSize), 0).xyz;
                return vec3(
                    (v.x >> 31u == 0u)?0.0:1.0,
                    (v.y >> 31u == 0u)?0.0:1.0,
                    (v.z >> 31u == 0u)?0.0:1.0);
                    ;
            }
        ''').format( X=functionName )  ) 

