import Gears as gears
from .. import * 


class Spot(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            radius          : 'Spot radius [um].'
                            = 200,
            innerRadius     : 'Annulus inner radius [um] (or negative for solid disc).'
                            = -1000,
            filterRadius_um : 'Antialiasing filter size [um] (shape blur).'
                            = 0.1
            ) :
        patch.setShaderVariable( name = 'spotRadius',    value = radius )
        patch.setShaderVariable( name = 'spotInnerRadius',    value = innerRadius )
        patch.setShaderVariable( name='filterRadius', value = filterRadius_um )
        patch.setShaderFunction( name = 'shape', src = '''
                vec3 shape(vec2 x){ 
                    float diff = length(x);
                    float inOrOut = (1-smoothstep( -filterRadius, +filterRadius, diff - spotRadius )) * (1-smoothstep( -filterRadius, +filterRadius, spotInnerRadius - diff ));
                    return vec3(inOrOut, inOrOut, inOrOut);
                }
        '''  )

