import Gears as gears
from .. import * 

class Filter(Component) : 
    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.setTemporalWeights( self.getTemporalWeights(stimulus), True )

    def getTemporalWeights(
            self,
            stimulus,
            ) :
        return self.getTemporalWeightsWithArgs(stimulus, **self.args)