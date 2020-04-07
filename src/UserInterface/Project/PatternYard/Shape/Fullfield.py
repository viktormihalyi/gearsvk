import Gears as gears
from .. import * 

class Fullfield(Component) : 

    def applyWithArgs(
            self,
            patch,
            ) :
        patch.setShaderFunction( name = 'shape', src = 'vec3 shape(vec2 x){ return vec3(1, 1, 1); }' )
