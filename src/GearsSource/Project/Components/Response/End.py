import Gears as gears
from .. import * 
import traceback
import inspect
from .Base import *

class End(Base) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(
            self,
            *,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'End'
           ):
        self.name = name
   



