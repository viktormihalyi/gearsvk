import Gears as gears
from .. import * 
from .Filter import *

class Hamming(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
            *,
            length          : 'Length of temporal filter in frames.'
                            =	16,
            memoryLength    : 'Number of previous frames considered.'
                            = 32   	     
            ) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setTemporalWeightingFunction(
            """
                float temporalWeight(int t)
                {{
                  if (t<{length}) return 0.54-0.46*cos(6.283118530718 * t/({length}-1));
                  else return 0.0;
                }}
            """.format(length=length),
            memoryLength,
            True,
            0 ,
            1 )
            





