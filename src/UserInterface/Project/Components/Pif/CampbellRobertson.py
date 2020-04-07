import Gears as gears
from .. import * 
from .Base import *

class CampbellRobertson(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color1          : '"bright" pattern color,  or Interactive.*'
                            = 'white',
            color2          : '"dark" pattern color,  or Interactive.*'
                            = 'black',
            startWavelength : 'Sine pattern wavelength [um] at screen edge.'
                            = 300,
            endWavelength   : 'Sine pattern wavelength [um] at screen edge.'
                            = 30,
            minContrast     : 'Contrast level at screen edge (0 - lowest, 1 - highest).'                            
                            = 0,
            maxContrast     : 'Contrast level at screen edge (0 - lowest, 1 - highest).'                            
                            = 1,
            sineExponent    : '<BR>The power the sine function is raised to (1 - sine wave, 0.01 - square wave),<\BR><BR> or Interactive.*'
                            = 1,
            contrastGradientExponent : '<BR>The exponent of the contrast vs. screen position curve (1 - linear),<\BR><BR> or Interactive.*'
                            = 1.7,
            direction       : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')"
                            = 'east'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)

        if not (isGrey(color1) and isGrey(color2)):
            spass.enableColorMode()
        direction = processDirection(direction, self.tb)

        stimulus = spass.getStimulus()
        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                color1 = color1,
                color2 = color2,
                sineExponent = sineExponent, 
                contrastGradientExponent = contrastGradientExponent,
                contrastLevels = (minContrast, maxContrast),
                direction = direction,
               )        

        #spass.setShaderColor( name = functionName+'_color1', red = color1[0], green=color1[1], blue=color1[2] )
        #spass.setShaderColor( name = functionName+'_color2', red = color2[0], green=color2[1], blue=color2[2] )

        
        try:
            direction.setDirection()
            s = math.sin(direction.value)
            c = math.cos(direction.value)
        except:
            s = math.sin(direction)
            c = math.cos(direction)
            direction = (c,s)

        sequence = stimulus.getSequence()

        patternLength = math.fabs(sequence.field_width_um * c) + math.fabs(sequence.field_height_um * s)
        patternWidth = math.fabs(sequence.field_height_um * c) + math.fabs(sequence.field_width_um * s)
        
        spass.setShaderVector( name=functionName+'_wavelength',
                             x = startWavelength / patternLength * 1.4142135623730950488016887242097,
                             y = endWavelength / patternLength * 1.4142135623730950488016887242097 )
        #spass.setShaderVector( name=functionName+'_contrastLevels', x=minContrast, y=maxContrast )
        spass.setShaderVector( name=functionName+'_span', x=c/patternLength, y=s/patternLength )
        spass.setShaderVector( name=functionName+'_cspan', x=c/patternWidth, y=s/patternWidth )
        #spass.setShaderVariable( name=functionName+'_sineExponent', value=sineExponent )
        #spass.setShaderVariable( name=functionName+'_contrastGradientExponent', value=contrastGradientExponent )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            vec3 @<X>@(vec2 x, float time){
                float t = dot(x,  @<X>@_span) + 0.5;
                float s = clamp(cross(vec3(x, 0), vec3( @<X>@_cspan, 0)).z + 0.5, 0, 1);
               	float q = log(-  @<X>@_wavelength.x * t +  @<X>@_wavelength.x +  @<X>@_wavelength.y * t) / ( @<X>@_wavelength.x +  @<X>@_wavelength.y);
        		q -= log( @<X>@_wavelength.x) / ( @<X>@_wavelength.x +  @<X>@_wavelength.y);
                if(abs( @<X>@_wavelength.x -  @<X>@_wavelength.y) < 0.000001)
                        q = t /  @<X>@_wavelength.x * 1.4142135623730950488016887242097;
        		float f = cos (q * 6.283185307179586476925286766559);
		        if(f>0)
			        f = pow(f,  @<X>@_sineExponent);
		        else
			        f = -pow(-f,  @<X>@_sineExponent);
                float ys;
		        if( @<X>@_contrastGradientExponent > 0)
			        ys = pow(s,  @<X>@_contrastGradientExponent);
		        else
			        ys = pow(1-s, - @<X>@_contrastGradientExponent);
		        ys = ys *  @<X>@_contrastLevels.y + (1-ys) *  @<X>@_contrastLevels.x;
                return mix( @<X>@_color1,  @<X>@_color2, 0.5 - f * ys * 0.5);
            }
        ''').format( X=functionName )  )

        def update(
            self,
            *,
           color1 = 'white',
           color2 = 'black',
           sineExponent = 1, 
           contrastGradientExponent = 1.7,
           minContrast = 0,
           maxContrast =1,
           direction = 'east'
               ):
        
           direction = processDirection(direction, self.tb)
           try:
               direction.setDirection()
               s = math.sin(direction.value)
               c = math.cos(direction.value)
           except:
               s = math.sin(direction)
               c = math.cos(direction)
               direction = (c,s)
        
           sequence = stimulus.getSequence()
        
           patternLength = math.fabs(self.sequence.field_width_um * c) + math.fabs(self.sequence.field_height_um * s)
           patternWidth = math.fabs(self.sequence.field_height_um * c) + math.fabs(self.sequence.field_width_um * s)
           self.spass.setShaderVector( name=self.functionName+'_wavelength',
                             x = self.startWavelength / patternLength * 1.4142135623730950488016887242097,
                             y = self.endWavelength / patternLength * 1.4142135623730950488016887242097 )
        
           self.spass.setShaderVector( name=self.functionName+'_span', x=c/patternLength, y=s/patternLength )
           self.spass.setShaderVector( name=self.functionName+'_cspan', x=c/patternWidth, y=s/patternWidth )
           Component.update(color1 = color1, 
                            color2 =color2, 
                            sineExponent = sineExponent, 
                            contrastGradientExponent =contrastGradientExponent, 
                            contrastLevels = (minContrast, maxContrast),
                            )