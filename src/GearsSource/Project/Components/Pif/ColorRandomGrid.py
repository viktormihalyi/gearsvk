import Gears as gears
from .. import * 
from .Base import *

class ColorRandomGrid(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            ) :
        #randoms and cellSize are set from the C++ DLL
        stimulus = spass.getStimulus()
        spass.enableColorMode()
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            #ifndef GEARS_RANDOMS_RESOURCES
            #define GEARS_RANDOMS_RESOURCES
            uniform usampler2D randoms;
            uniform vec2 cellSize;
            uniform ivec2 randomGridSize;
            #endif
            vec3 @<X>@ (vec2 x, float time){ 
                ivec2 iv = ivec2((x + randomGridSize * cellSize * 0.5) / cellSize);
                vec3 v = vec3(texelFetch(randoms, iv , 0).xyz) / float(0xffff) / float(0xffff);
                return v;
            }
        ''').format( X=functionName )  ) 

