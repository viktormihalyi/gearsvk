import Gears as gears
from .. import * 


class Gaussian(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            maxValue        : 'Gaussian function peak [um].'
                            = 1,
            variance        : 'Gaussian variance [um].'
                            = 200
            ) :
        patch.setShaderVariable( name = 'gaussianVariance',    value = variance )
        patch.setShaderVariable( name = 'gaussianPeak',    value = maxValue )
        patch.setShaderFunction( name = 'shape', src = '''
                vec3 shape(vec2 x){ 
                    float diff = length(x);
                    float g = maxValue * exp( - diff * diff / variance * 0.5 );
                    return vec3(g, g, g);
                }
        '''  )

