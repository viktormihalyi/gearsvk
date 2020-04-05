import Gears as gears
from .. import * 

class BandPass(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            r       : 'inner radius'
                    = 5,
            R       : 'outer radius'
                    = 10
           
            ) :
        self.kernelGivenInFrequencyDomain = True
        seq = stimulus.getSequence()

        stimulus.setSpatialFilter(self)
        self.registerInteractiveControls(
                self, stimulus,
                '',
                r=r,
                R=R,
                )

        #self.showFft = True
        pixelScale = -seq.fft_width_px * seq.fft_height_px / seq.getSpatialFilteredFieldWidth_um() / seq.getSpatialFilteredFieldHeight_um();
        #self.setShaderVariable("r", r )
        #self.setShaderVariable("R",R )
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                float l = length(x);
                if(  (l < R) && (l>r) )
                      return vec4(1, 0, 1, 0);
                else
                    return vec4(0, 0, 0, 0);
               }
        """)

       
    def update(
        self,
        *,
        R = 1000,
        r = 10):
        Component.update(self, 
                         R = R,
                         r =r
                         )
    
        self.width_um = R * 2
        self.height_um = R * 2
        self.maximum = 1
        self.minimum = 0
        gears.updateSpatialKernel()


