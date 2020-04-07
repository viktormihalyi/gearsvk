import Gears as gears
from .. import * 

class IdealLowpass2(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            radius_ium  : 'TODO! Frequency domain disc function radius [1/um].'
                        = 50.0
            ) :
        self.kernelGivenInFrequencyDomain = True
        #self.setShaderVariable("radius_um", radius_um )
        #self.setShaderFunction("kernel", """
        #    vec4 kernel(vec2 x){
        #        float l = length(x);
        #        if(  l < radius_um )
        #        {
        #            float sinc = sin(l * 0.10)/ (l * 0.10);
        #            return vec4(sinc, 0, sinc, 0) * 0.01;
        #        }
        #        else
        #            return vec4(0, 0, 0, 0);
        #        }
        #""")
        #self.setShaderFunction("kernel", """
        #    vec4 kernel(vec2 x){
        #        float sigma = 50.0;
        #        float weight = 10000000.000001;
        #        float a = exp(-dot(x,x) / (2 * sigma * sigma)) / (sigma * 6.283185307179586476925286766559 * sigma);
        #        float s = a * weight;
        #        return vec4(s, 0, s, 0); }
        #""")

        sigma1  = 200/50.0
        weight1 = -1000000.0
        sigma2  = 200/50.0
        weight2 = 000000.0
        offset = 0
        self.setShaderVariable("sigma1", sigma1 )
        self.setShaderVariable("weight1",weight1)
        self.setShaderVariable("sigma2", sigma2 )
        self.setShaderVariable("weight2",weight2)
        self.setShaderVariable("offset", offset )
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                vec2 sigma = vec2(sigma1, sigma2);
                vec2 weight = vec2(weight1, weight2);
                vec2 a = exp(-dot(x,x) / (2 * sigma * sigma)) / (sigma * 6.283185307179586476925286766559 * sigma);
                float s = dot(a, weight) + offset;
                return vec4(s, 0, s, 0); }
        """)
        #self.width_um = radius_um * 2
        #self.height_um = radius_um * 2
        #self.maximum = 1
        #self.minimum = self.maximum * -0.125
        #stimulus.setSpatialFilter(self)
       
        self.width_um = sigma1 * 5
        self.height_um = sigma1 * 5
        self.maximum = weight1 / (sigma1 * 6.283185307179586476925286766559 * sigma1) + weight2 / (sigma2 * 6.283185307179586476925286766559 * sigma2)
        self.minimum = self.maximum * -0.125
        stimulus.setSpatialFilter(self)


