import Gears as gears
from .. import * 
from .SingleShape import *

class FullfieldOscillation(SingleShape) :

    def boot(self,
            *,  
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'FF oscillation',
            wavelength_s    : 'Modulation wavelength [s].'
                            = 0,
            **bargs         : Modulation.Cosine
            ):
        if name == 'FF oscillation' and wavelength_s != 0:
            name = '{freq:.0f} Hz'.format( freq = 1 / wavelength_s )
        super().boot(name=name, duration=duration, duration_s=duration_s,
                modulation = Modulation.Cosine( wavelength_s=wavelength_s, **bargs ),
                )