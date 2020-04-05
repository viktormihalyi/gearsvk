import Gears as gears
from .. import * 
from .Base import *

class GreyscaleRandomGrid(Base) : 

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
                float v = float(texelFetch(randoms, ivec2( (x + randomGridSize * cellSize * 0.5) / cellSize), 0).x) / float(0xffff) / float(0xffff);
                return vec3(v, v, v);

            }
        ''').format( X=functionName )  ) 

