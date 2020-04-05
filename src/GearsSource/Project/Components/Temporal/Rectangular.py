import Gears as gears
from .. import * 
from .Filter import *

class Rectangular(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
            *,
            length          : 'Length of temporal filter in frames.'
                            =	16	     ,
            a               : 'Weight of current frame (frame 0).'
                            =	0.1         ,
            memoryLength    : 'Number of previous frames considered.'
                            = 32            ) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setTemporalWeightingFunction(
            """
                float temporalWeight(int t)
                {{
                  if (t<{length}) return float({a});
                  else return 0.0;
                }}
            """.format(length=length, a=a),
            memoryLength,
            True,
            0,
            a
            )



