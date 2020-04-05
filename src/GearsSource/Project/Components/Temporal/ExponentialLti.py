import Gears as gears
from .. import * 
from .Filter import *

class ExponentialLti(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
            *,
            q               : 'Quotient of weight between frames.'
                            =	0.95	     ,
            a               : 'Weight of current frame (frame 0).'
                            =	1/40
            ) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setLtiMatrix(
            [
            0.05, 0.95, 0, 0,
            0.05, 0.95, 0, 0,
            0, 0, 0, 0,
            0.0, 0.0, 0, 0,
            ]
            )
