import Gears as gears
from .. import * 

class DisplayStimFft(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            ) :
        self.kernelGivenInFrequencyDomain = True
        self.showFft = True
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                return vec4(1, 0, 1, 0); 
                }
        """)
       
        self.width_um = 0
        self.height_um = 0
        self.maximum = 1.1
        self.minimum = 0.9
        stimulus.setSpatialFilter(self)


