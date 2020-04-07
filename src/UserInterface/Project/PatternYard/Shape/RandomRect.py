import Gears as gears
from .. import * 
import math

class RandomRect(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            size_um             : 'Dimensions of the rectangle shape, as a (width, height) pair [um,um].'
                                = (2000, 200),
            follow_distance_um  : 'Distance between instances of the rectangle along the width axis [um].'
                                = 100000000,
            wingmen_distance_um : 'Distance between instances of the rectangle along the height axis [um].'
                                = 100000000,
            filterRadius_um     : 'Antialiasing filter size [um] (shape blur).'
                                = 0.1
            ) :

        patch.setShaderVector( name = 'rect',    x = size_um[0], y= size_um[1] )
        patch.setShaderVector( name='repetitionDistance', x=follow_distance_um, y=wingmen_distance_um )
        patch.setShaderVector( name='rect', x=size_um[0], y=size_um[1] )
        patch.setShaderVariable( name='filterRadius', value = filterRadius_um )
        patch.setShaderFunction( name = 'shape', src = '''
                uniform usampler2D randoms;
                vec3 shape(vec2 x){ 
                    float r = float(texelFetch(randoms, ivec2(0, 0), 0).x) * 6.28 / 0xffffffff;
                    vec2 facing = vec2(cos(r), sin(r));
                    vec2 rotatedX = vec2( x.x * facing.x + x.y * facing.y, x.x * facing.y - x.y * facing.x);
                    rotatedX = mod(rotatedX + repetitionDistance*0.5 , repetitionDistance) - repetitionDistance * 0.5;
                    float xDiff = abs(rotatedX.x) - rect.x * 0.5;
                    float yDiff = abs(rotatedX.y) - rect.y * 0.5;
                    float inOrOut = (1-smoothstep( -filterRadius, +filterRadius, xDiff )) * (1-smoothstep( -filterRadius, +filterRadius, yDiff ));
                    return vec3(inOrOut, inOrOut, inOrOut);
                    }
        '''  )





