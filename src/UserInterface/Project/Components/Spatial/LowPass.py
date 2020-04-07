import Gears as gears
from .. import * 

class LowPass(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            r       : 'radius'
                    = 0.5
           
            ) :
        self.kernelGivenInFrequencyDomain = True
        seq = stimulus.getSequence()
        #self.showFft = True
        pixelScale = -seq.fft_width_px * seq.fft_height_px / seq.getSpatialFilteredFieldWidth_um() / seq.getSpatialFilteredFieldHeight_um();
        self.setShaderVariable("r", r )
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                float l = length(x);
                if(  l < r )
                      return vec4(100, 0, 100, 0);
                else
                    return vec4(0, 0, 0, 0);
               }
        """)
        #self.width_um = radius_um * 2
        #self.height_um = radius_um * 2
        #self.maximum = 1
        #self.minimum = self.maximum * -0.125
        #stimulus.setSpatialFilter(self)
       
        self.width_um = r * 2
        self.height_um = r * 2
        self.maximum = 1
        self.minimum = 0
        stimulus.setSpatialFilter(self)


