import Gears as gears
from .. import * 
import traceback
from .SinglePass import *

class ExtApollonian(SinglePass) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Apollonian'
            ):
        super().boot(name=name, duration=duration, duration_s=duration_s,
                    spass = Pass.ExtApollonian(
                    ),
            )

