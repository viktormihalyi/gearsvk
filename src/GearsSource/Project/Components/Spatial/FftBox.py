import Gears as gears
from .. import * 

class FftBox(Component, gears.SpatialFilter) :

    def __init__(self, **args):
        gears.SpatialFilter.__init__(self)
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            *,
            halfEdge_um     : "Half of the spatial domain box function's hold length [um]."
                            = 50.0
            ) :

        #self.kernelGivenInFrequencyDomain = False;

        stimulus.setSpatialFilter(self)
        self.registerInteractiveControls(
            self, stimulus,
            "",
            halfEdge_um = halfEdge_um,
            )
        #self.setShaderVariable("halfEdge_um", halfEdge_um )
        self.setShaderFunction("kernel", """
            vec4 kernel(vec2 x){
                if(abs(x.x) < halfEdge_um && abs(x.y) < halfEdge_um)
                    return vec4(0.25 / (halfEdge_um*halfEdge_um), 0, 0, 0);
                else
                return vec4(0, 0, 0, 0);
                }
        """)

    def update(
        self,
        *,
        halfEdge_um =1):
        Component.update(self,
                         halfEdge_um = halfEdge_um)
        self.width_um = halfEdge_um * 4
        self.height_um = halfEdge_um * 4
        self.maximum = 0.25 / (halfEdge_um*halfEdge_um)
        self.minimum = self.maximum * -0.125
        gears.updateSpatialKernel()




