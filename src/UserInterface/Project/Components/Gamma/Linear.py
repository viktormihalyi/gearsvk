import Gears as gears
from .. import * 

class Linear(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.setGamma( [0, 1] )    
        stimulus.gammaLabel = 'linear'      

