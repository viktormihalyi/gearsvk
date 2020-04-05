import Gears as gears
from .. import * 

class DogFftSpatSpec(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            sigma1  : 'Standard deviation of 2D Gaussian term 1 [um].'
                    = 8.0,
            weight1 : 'Weight factor for 2D Gaussian term 1, used when adding the two Gaussians.'
                    = -100.0,
            sigma2  : 'Standard deviation of 2D Gaussian term 2 [um].'
                    = 5.0,
            weight2 : 'Weight factor for 2D Gaussian term 2, used when adding the two Gaussians.'
                    = 100.0,
            offset  : 'Constant term added to the weighted sum of the two Gaussians.'
                    = 0.0
            ) :

        #self.showFft = True;
        self.kernelGivenInFrequencyDomain = False;

        stimulus.setSpatialFilter(self)
        self.registerInteractiveControls(
            self, stimulus,
            "",
            sigma1  = sigma1   ,
            weight1 = weight1  ,
            sigma2  = sigma2   ,
            weight2 = weight2  ,
            offset  = offset   ,
            )
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                vec2 sigma = vec2(sigma1, sigma2);
                vec2 weight = vec2(weight1, weight2);
                vec2 a = exp(-dot(x,x) / (2 * sigma * sigma)) / (sigma * 6.283185307179586476925286766559 * sigma);
                float s = dot(a, weight) + offset;
                return vec4(s, 0, s, 0); }
        """)

    def update(
            self,
            *,
            sigma1 = 0.1,
            weight1 = 0,
            sigma2 = 0.1,
            weight2 = 0,
            offset = 0):
        Component.update(self, 
                         sigma1=sigma1, 
                         weight1=weight1, 
                         sigma2=sigma2, 
                         weight2=weight2, 
                         offset=offset, 
                         )
        #self.setShaderVariable(name="sigma1", value=sigma1)
        #self.setShaderVariable(name="weight1", value=weight1)
        #self.setShaderVariable(name="sigma2", value=sigma2)
        #self.setShaderVariable(name="weight2", value=weight2)
        #self.setShaderVariable(name="offset", value=offset)

        self.width_um = sigma1 * 5
        self.height_um = sigma1 * 5
        self.maximum = weight1 / (sigma1 * 6.283185307179586476925286766559 * sigma1) + weight2 / (sigma2 * 6.283185307179586476925286766559 * sigma2)
        self.minimum = self.maximum * -0.125
        gears.updateSpatialKernel()
     

