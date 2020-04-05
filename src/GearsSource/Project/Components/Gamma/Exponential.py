import Gears as gears
from .. import * 

class Exponential(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            exponent    : 'Exponent for gamma function.'
                        = 1
            ) :
                       
        gammas = []
        for i in range(0, 101) :
            gammas.append( math.pow(i / 100, exponent) )
        stimulus.setGamma( gammas )
        stimulus.gammaLabel = 'V^{expo}'.format( expo = exponent)


