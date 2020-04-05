import Gears as gears
from .. import * 

class SmallDog(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            sigma1  : 'Standard deviation of 2D Gaussian term 1 [um].'
                    = 80.0,
            weight1 : 'Weight factor for 2D Gaussian term 1, used when adding the two Gaussians.'
                    = -10.0,
            sigma2  : 'Standard deviation of 2D Gaussian term 2 [um].'
                    = 50.0,
            weight2 : 'Weight factor for 2D Gaussian term 2, used when adding the two Gaussians.'
                    = 10.0,
            offset  : 'Constant term added to the weighted sum of the two Gaussians.'
                    = 0.0
            ) :
        self.setShaderVariable("sigma1", sigma1 )
        self.setShaderVariable("weight1",-weight1)
        self.setShaderVariable("sigma2", sigma2 )
        self.setShaderVariable("weight2",weight2)
        self.setShaderVariable("offset", offset )
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                vec2 sigma = vec2(sigma1, sigma2);
                vec2 weight = vec2(weight1, weight2);
                vec2 a = exp(-dot(x,x) / (2 * sigma * sigma)) / sqrt(sigma * 6.283185307179586476925286766559 * sigma);
                vec2 s = a * weight;
//                    return (x.y < 0)?vec4(1, 0, 1, -1):vec4(1, 0, -1, 1); }
              return vec4(s.xy, s.xy); }
//                return vec4(10, 10, 10, 10); }
        """)
        self.width_um = sigma1 * 5
        self.height_um = sigma1 * 5
        self.maximum = weight1 / (sigma1 * 6.283185307179586476925286766559 * sigma1) + weight2 / (sigma2 * 6.283185307179586476925286766559 * sigma2)
        self.minimum = self.maximum * -0.125
        self.useFft = False
        self.separable = True
        self.horizontalSampleCount = 17
        self.verticalSampleCount = 17
        stimulus.setSpatialFilter(self)
