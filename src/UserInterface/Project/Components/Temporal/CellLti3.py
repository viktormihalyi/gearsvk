import Gears as gears
from .. import * 
from .Filter import *

class CellLti3(Filter) : 

  def applyWithArgs(
            self,
            stimulus,
) :
        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setLtiMatrix(
            [
             0       ,  0.477349, -0.0705042, 0.103746,
             0.383115,  0.865133,  0.366234, -0.100492,
             0.330595, -0.366234,  0.768675,  0.175282,
            -0.247068,  0.100492,  0.175282,  0.810277,
            ]
            )
