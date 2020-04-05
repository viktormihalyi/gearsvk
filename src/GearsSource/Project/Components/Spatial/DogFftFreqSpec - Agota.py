import Gears as gears
from .. import * 

class DogFftFreqSpec(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            sigma1  : 'Standard deviation of 2D Gaussian term 1 [um]. Frequency domain sigma1 is computed from this as 1/(2 pi sigma1).'
                    = 5.0,
            weight1 : 'Weight factor for 2D Gaussian term 1, used when adding the two Gaussians. This is the integral of the Gaussian in the spatial domian.'
                    = -10.0,
            sigma2  : 'Standard deviation of 2D Gaussian term 2 [um]. Frequency domain sigma1 is computed from this as 1/(2 pi sigma2).'
                    = 10.0,
            weight2 : 'Weight factor for 2D Gaussian term 2, used when adding the two Gaussians. This is the integral of the Gaussian in the spatial domian.'
                    = 10.0,
            offset  : 'Constant term added to the weighted sum of the two Gaussians. This is added directly in the frequency domain.'
                    = 0.0
            ) :
        self.kernelGivenInFrequencyDomain = True

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
        
        seq = stimulus.getSequence()
        pixelScale = -seq.fft_width_px * seq.fft_height_px / seq.getSpatialFilteredFieldWidth_um() / seq.getSpatialFilteredFieldHeight_um();
       
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                vec2 sigma = vec2(sigma1, sigma2);
                vec2 weight = vec2(weight1, weight2);
                vec2 a = exp(-dot(x , x ) / (2 * sigma * sigma));
                float s = dot(a, weight) + offset;
                return vec4(s, 0, s, 0); }
        """)
        #self.width_um = radius_um * 2
        #self.height_um = radius_um * 2
        #self.maximum = 1
        #self.minimum = self.maximum * -0.125
        #stimulus.setSpatialFilter(self)
    def update(
            self,
            *,
            sigma1 = 0.1,
            weight1 = 0,
            sigma2 = 0.1,
            weight2 = 0,
            offset = 0,
            pixelScale =1):

        sigma1 = 1/6.283185307179586476925286766559/sigma1 
        weight1 = weight1 * pixelScale
        sigma2 = 1/6.283185307179586476925286766559/sigma2 
        weight2 = weight2 * pixelScale
        offset = -offset 
        Component.update(self, 
                         sigma1=sigma1, 
                         weight1=weight1, 
                         sigma2=sigma2, 
                         weight2=weight2, 
                         offset=offset, 
                         )
       
        self.width_um = sigma1 * 5 + 20
        self.height_um = sigma1 * 5 + 20
        self.maximum = weight1 / (sigma1 * 6.283185307179586476925286766559 * sigma1) + weight2 / (sigma2 * 6.283185307179586476925286766559 * sigma2)
        self.minimum = self.maximum * -0.125
        gears.updateSpatialKernel()


