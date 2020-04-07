import Gears as gears
from .. import * 
from .SingleShape import *

class FullfieldGradient(SingleShape) :

    def boot(self,
            *,  
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'gradient',
            toneMapping     : 'Tone mapping component (Tone.*)'
                            = Tone.UiConfigured(),
            **bargs         : Pif.Gradient 
            ):

        super().boot(name=name, duration=duration, duration_s=duration_s,
                pattern = Pif.Gradient( **bargs ),
                toneMapping = toneMapping,
                )