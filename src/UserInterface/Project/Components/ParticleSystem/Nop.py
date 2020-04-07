import Gears as gears
from .. import * 

class Nop(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.particleGridWidth =  0
        stimulus.particleGridHeight = 0
