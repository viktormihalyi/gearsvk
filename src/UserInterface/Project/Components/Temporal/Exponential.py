import Gears as gears
from .. import * 
from .Filter import *

class Exponential(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
            *,
            q               : 'Quotient of weight between frames.'
                            =	0.95	     ,
            a               : 'Weight of current frame (frame 0).'
                            =	1/40         ,
            memoryLength    : 'Number of previous frames considered.'
                            = 32            ) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setTemporalWeightingFunction(
            """
                float temporalWeight(int t)
                {{
                    return
                        pow( {q}, float(t) ) * {a};
                }}
            """.format(q=q, a=a),
            memoryLength,
            True,
            0,
            a
            )
