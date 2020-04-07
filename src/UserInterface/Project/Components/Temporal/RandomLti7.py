import Gears as gears
from .. import * 
from .Filter import *

class RandomLti7(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setLtiImpulseResponse(
            [
                0.1*0,
                0.1*0.5,
                0.1*0.1,
                0.1*-0.2,
                0.1*-0.5,
                0.1*0.9,
                0.1*0.9,
                0.1*0.5,
                0.1*0.1,
                0.1*-0.5,
                0.1*0.4,
                0.1*0.2,
                0.1*0.5,
                0.1*0.2,
                0.1*0.3,
                0.1*0.1,
            ],
            7
            )
