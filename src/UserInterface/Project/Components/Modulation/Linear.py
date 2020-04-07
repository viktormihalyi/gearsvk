import Gears as gears
from .. import * 
from .Base import *

class Linear(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            displayName     : 'The name that appears on the experience timeline plot. If None, this temporal modulation function is not displayed. '
                        = None,
            brightColor : 'Modulating color at unit modulated intensity. (r,g,b) triplet or color name. (I)'
                        = (1, 1, 1),
            darkColor   : 'Modulating color at zero modulated intensity. (r,g,b) triplet or color name. (I)'
                        = (0, 0, 0),
            intensity   : 'Initial intensity (in [0,1], or "unspecified" to find value using intensitySlope)'
                        = 'unspecified',
            intensitySlope : 'Intensity change over time [1/s]. "hold" means 0, with intensity 1 unless otherwise specified. "down" means intensity goes from 1 to 0 in duration. "up" means 0 to 1.'
                            = 'hold'
            ) :

        if displayName:
            spass.registerTemporalFunction(functionName, displayName)

        brightColor = processColor(brightColor, self.tb)
        darkColor = processColor(darkColor, self.tb)

        stimulus = spass.getStimulus()
        if not isGrey(brightColor) or not isGrey(darkColor):
            spass.enableColorMode()

        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                brightColor     =   brightColor,
                darkColor       =   darkColor,
                )

        duraSec = spass.getDuration_s()
        intensityBase = intensity
        if intensitySlope == 'hold' :
            intensitySlope = 0
            if intensityBase == 'unspecified' :
                intensityBase = 1
        elif intensitySlope == 'down' :
            intensitySlope = -1 / duraSec
            if intensityBase == 'unspecified' :
                intensityBase = 1
        elif intensitySlope == 'up' :
            intensitySlope = 1 / duraSec
            if intensityBase == 'unspecified' :
                intensityBase = 0
        else:
            intensitySlope = intensitySlope
            if intensityBase == 'unspecified' :
                intensityBase = 1

        try:
            intensityBase = float(intensityBase)    
        except:
            raise ValueError('Argument "intensity" has invalid value "{intensityBase}"'.format(intensityBase=intensityBase) )
        try:
            intensitySlope = float(intensitySlope)    
        except:
            raise ValueError('Argument "intensitySlope" has invalid value "{intensitySlope}"'.format(intensitySlope=intensitySlope) )
        spass.setShaderVector( name = functionName + "_intensityLinearFactors", x = intensityBase, y = intensitySlope )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            vec3 @<X>@(float time){ return mix(@<X>@_darkColor, @<X>@_brightColor, @<X>@_intensityLinearFactors.x + time * @<X>@_intensityLinearFactors.y); }
        ''' ).format( X = functionName ) )
