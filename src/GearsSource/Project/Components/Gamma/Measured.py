import Gears as gears
from .. import * 

class Measured(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            measuredCurve   :  'List of at most 101 samples of the gamma curve, at regular distances.'
                            =  [0,   0.31, 0.415, 0.485, 0.535, 0.58, 0.605, 0.63,  0.66,  0.677, 0.705,   0.725,   0.74,  0.757,   0.774,  0.79,   0.815,  0.845,  0.888,  0.915,  1],
            invert          : 'If True, invert the given gamma function to get the correction function to be applied.'
                            = False
            ) :
        stimulus.setGamma( measuredCurve, invert=invert )
        stimulus.gammaLabel = 'measured'


