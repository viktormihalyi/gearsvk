import Gears as gears
from .. import * 
from .SingleShape import *

class SpotOscillation(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'SpotOscillation',
            brightColor     : 'Brightest color possible. (r,g,b) triplet or color name.'
                            = (1, 1, 1),
            darkColor       : 'Darkest color possible. (r,g,b) triplet or color name.'
                            = (0, 0, 0),
            intensity       : 'Base intensity before modulation'
                            = 0.5,
            wavelength_s    : 'Starting wavelength [s].'
                            = 0,
            endWavelength_s : 'Ending wavelength [s].'
                            = 0,
            exponent        : 'Cosine exponent (1 for cosine, 0.01 for square signal).'
                            = 1,
            amplitude       : 'Starting intensity modulation amplitude (intensity will be in [intensity-amplitude, intensity+amplitude], starting at the minimum).'
                            = 0.5,
            endAmplitude    : 'Ending intensity modulation amplitude (defaults to amplitude). Amplitude is linearly interpolated over time.'
                            = -1,
            radius          : 'Disc radius [um].'
                            = 200,
            innerRadius     : 'Annulus hole radius [um]. Zero for solid disc.'
                            = -1000,
            position        : 'Spot center position as an (x,y) pair [(um, um)]. (0,0) is the field center.'
                            =   (0, 0),
            filterRadius_um : 'Antialiasing filter size [um].'
                            = 0.1,
            motion          : "Motion component. (Motion.*) If specified, parameter 'position' is ignored."
                            = None,         #TODO actual default uses parameter 'position'
            temporalFilter  : 'Temporal filter component. (Temporal.*)'
                            = Temporal.Nop()
            ):
        if name == 'SpotOscillation' :
            if endWavelength_s == 0 :
                name = 'Spot {a:.2f} Hz'.format( a = 1/wavelength_s )
            else:
                name = 'Spot {a:.2f}--{b:.2f} Hz'.format( a = 1/wavelength_s, b = 1/endWavelength_s )
        if motion == None :
            motion = Motion.Linear( 
                        startPosition=position,
                        )

        super().boot(name=name, duration=duration, duration_s=duration_s,
                modulation = Modulation.Cosine( 
                        brightColor      = brightColor    ,
                        darkColor        = darkColor      ,
                        intensity        = intensity      ,
                        wavelength_s     = wavelength_s   ,
                        endWavelength_s  = endWavelength_s,
                        exponent         = exponent       ,
                        amplitude        = amplitude      ,
                        endAmplitude     = endAmplitude   ,
                        ),
                shape = Pif.Spot(
                        radius          = radius          ,
                        innerRadius     = innerRadius     ,
                        filterRadius_um = filterRadius_um ,
                        ),
                shapeMotion = motion,
                temporalFilter = temporalFilter,
                )