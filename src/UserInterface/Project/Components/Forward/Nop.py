import Gears as gears
from .. import * 

class Nop() : 
    args = None

    def __init__(self, **args):
        self.args = args

    def apply(self, stimulus) :
        self.applyWithArgs(stimulus, **self.args)

    def applyWithArgs(
            self,
            stimulus,
            ) :
        pass