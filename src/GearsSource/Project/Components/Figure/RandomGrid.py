import Gears as gears
from .. import *
from .Base import * 

class RandomGrid(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            ) :
        #randoms and cellSize are set from the C++ DLL
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            #ifndef GEARS_RANDOMS_RESOURCES
            #define GEARS_RANDOMS_RESOURCES
            uniform usampler2D randoms;
            uniform vec2 cellSize;
            uniform ivec2 randomGridSize;
            #endif
            vec3 @<X>@ (vec2 x, float time){ 
                if(texelFetch(randoms, ivec2( (x + randomGridSize * cellSize*0.5 ) / cellSize ) , 0).x >> 31u == 0u)
        		    return vec3(0, 0, 0);
	            else
		            return vec3(1, 1, 1);
            }
        ''').format( X=functionName )  ) 

