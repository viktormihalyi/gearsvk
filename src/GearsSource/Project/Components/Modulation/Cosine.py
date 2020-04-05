import Gears as gears
from .. import * 
from .Base import *

class Cosine(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            displayName     : 'The name that appears on the experience timeline plot. If None, this temporal modulation function is not displayed. '
                            = None,
            brightColor     : 'Modulating color at unit modulated intensity. (I)'
                            = (1, 1, 1),
            darkColor       : 'Modulating color at zero modulated intensity. (I)'
                            = (0, 0, 0),
            intensity       : 'Base intensity before modulation. (I)'
                            = 0.5,
            wavelength_s    : 'Starting wavelength [s].'
                            = 0,
            endWavelength_s : 'Ending wavelength [s].'
                            = 0,
            exponent        : 'Cosine exponent (1 for cosine, 0.01 for square signal). (I)'
                            = 1,
            phase           : 'Phase shift [radian]. (I)'
                            = 0,
            amplitude       : 'Starting intensity modulation amplitude (intensity will be in [intensity-amplitude, intensity+amplitude], starting at the minimum).'
                            = 0.5,
            endAmplitude    : 'Ending intensity modulation amplitude (defaults to amplitude). Amplitude is linearly interpolated over time.'
                            = -1,
            linearFrequencyChange   : 'If False, wavelength changes linearly. If True, frequency changes linearly.'
                            = False
            ) :

        if displayName:
            spass.registerTemporalFunction(functionName, displayName)

        brightColor = processColor(brightColor, self.tb)
        darkColor = processColor(darkColor, self.tb)

        stimulus = spass.getStimulus()
        if not isGrey(brightColor) or not isGrey(darkColor):
            spass.enableColorMode()

        duraSec = spass.getDuration_s()

        spass.setShaderVariable(    name = functionName+'_duration_s', value = duraSec)
        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                exponent    = exponent,
                offset      = intensity,
                phase       = phase,
                brightColor = brightColor,
                darkColor   = darkColor,
                )
        spass.setShaderVector(      name = functionName+'_wavelength', x = wavelength_s / duraSec, y = (endWavelength_s / duraSec if endWavelength_s > 0 else wavelength_s / duraSec) )
        spass.setShaderVector(      name = functionName+'_amplitude', x = amplitude, y = (0 if endAmplitude < -0.1 else ((endAmplitude - amplitude) / duraSec)) )

        if linearFrequencyChange:        
            spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@(float time){ 
                    float t = time / `duration_s;
                    float q = log(- `wavelength.x * t + `wavelength.x + `wavelength.y * t) / (`wavelength.x + `wavelength.y);
        		    q -= log(`wavelength.x) / (`wavelength.x + `wavelength.y);
                    if(abs(`wavelength.x - `wavelength.y) < 0.000001)
                        q = time / `wavelength.x / `duration_s;
                    float currentAmplitude = `amplitude.x + time * `amplitude.y;
                    float s = cos(q * q * 6.283185307179586476925286766559 * 0.5+ `phase);
                    if(s<0)
                        s = -pow(-s, `exponent);
                    else
                        s = pow(s, `exponent);
                    return mix(`darkColor, `brightColor, -s*currentAmplitude + `offset);
                    }
            ''' ).format( X = functionName ) )
        else:
            spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@(float time){ 
                    float t = time / `duration_s;
                    float q = log(- `wavelength.x * t + `wavelength.x + `wavelength.y * t) / (`wavelength.x + `wavelength.y);
        		    q -= log(`wavelength.x) / (`wavelength.x + `wavelength.y);
                    if(abs(`wavelength.x - `wavelength.y) < 0.000001)
                        q = time / `wavelength.x / `duration_s;
                    float currentAmplitude = `amplitude.x + time * `amplitude.y;
                    float s = cos(q * 6.283185307179586476925286766559 + `phase);
                    if(s<0)
                        s = -pow(-s, `exponent);
                    else
                        s = pow(s, `exponent);
                    return mix(`darkColor, `brightColor, -s*currentAmplitude + `offset);
                    }
            ''' ).format( X = functionName ) )
