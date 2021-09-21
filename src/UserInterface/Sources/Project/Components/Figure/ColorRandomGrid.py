import GearsModule as gears
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
        stimulus.enableColorMode()
        spass.setShaderFunction( name = functionName, src = self.glslEsc( gears.GetGLSLResourcesForRandoms (stimulus) + '''
            vec3 @<X>@ (vec2 x, float time){ 
                ivec2 iv = ivec2((x + randomGridSize * cellSize * 0.5) / cellSize);
                vec3 v = vec3(randoms[randoms_layerIndex][iv.y][iv.x].xyz) / float(0xffff) / float(0xffff);
                return v;
            }
        ''').format( X=functionName )  ) 

