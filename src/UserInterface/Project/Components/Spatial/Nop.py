import Gears as gears
from .. import * 

class Nop(Component) :

    def __init__(self, **args):
        Component.__init__(self, **args)

    def applyWithArgs(
            self,
            stimulus,
            ) :
        pass