import Gears as gears
from .. import * 

class Nop(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.randomGridWidth =  0
        stimulus.randomGridHeight = 0
        stimulus.randomSeed = 0
