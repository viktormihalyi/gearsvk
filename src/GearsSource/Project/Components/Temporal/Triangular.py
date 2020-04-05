import Gears as gears
from .. import * 
from .Filter import *

class Triangular(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
            *,
            up              : 'Length of temporal filter in frames.'
                            =	8	     ,
            down            : 'Length of temporal filter in frames.'
                            =	8	     ,
            max             : 'Heigth of the triangle.'
                            =	0.1      ,
            memoryLength    : 'Number of previous frames considered.'
                            = 32            ) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setTemporalWeightingFunction(
            """
                float temporalWeight(int t)
                {{
                  if (t<{up}) return float({max}*t/{up});
                  else if (t<{up}+{down}) return ({max}-{max}*(t-{up})/{down});
                }}
            """.format(up=up, down=down, max=max),
            memoryLength,
            True,
            0,
            max
            )



