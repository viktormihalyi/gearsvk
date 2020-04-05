import Gears as gears
from .. import * 
from .Filter import *

class Nop(Filter) : 

    def applyWithArgs(
            self,
            stimulus,
            ) :
        pass

    def getTemporalWeightsWithArgs(self, stimulus) :
        return [1]
