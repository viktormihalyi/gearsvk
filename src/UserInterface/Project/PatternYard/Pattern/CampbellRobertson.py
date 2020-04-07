import Gears as gears
from .. import * 

class CampbellRobertson(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            color1          : '"bright" pattern color'
                            = 'white',
            color2          : '"dark" pattern color'
                            = 'black',
            background      : 'Color outside of shape.'
                            = 'black',
            startWavelength : 'Sine pattern wavelength [um] at screen edge.'
                            = 300,
            endWavelength   : 'Sine pattern wavelength [um] at screen edge.'
                            = 30,
            minContrast     : 'Contrast level at screen edge (0 - lowest, 1 - highest).'
                            = 0,
            maxContrast     : 'Contrast level at screen edge (0 - lowest, 1 - highest).'
                            = 1,
            sineExponent    : 'The power the sine function is raised to (1 - sine wave, 0.01 - square wave).'
                            = 1,
            contrastGradientExponent : 'The exponent of the contrast vs. screen position curve (1 - linear).'
                            = 1.7,
            direction       : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')"
                            = 'east',
            patternFunctionName : 'The function given by this pattern (some stimuli use multiple patterns).'
                            = 'pattern'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)
        background = processColor(background, self.tb)

        stimulus = patch.getStimulus()

        if max(color1) - min(color1) > 0.03 or max(color2) - min(color2) > 0.03 or max(background) - min(background) > 0.03:
            stimulus.enableColorMode()

        patch.setShaderColor( name = 'color1', red = color1[0], green=color1[1], blue=color1[2] )
        patch.setShaderColor( name = 'color2', red = color2[0], green=color2[1], blue=color2[2] )
        patch.setShaderColor( name = 'background', red = background[0], green=background[1], blue=background[2] )

        direction = processDirection(direction, self.tb)
        experiment = stimulus.getExperiment()
        s = math.sin(direction)
        c = math.cos(direction)
        patternLength = math.fabs(experiment.field_width_um * c) + math.fabs(experiment.field_height_um * s)
        patternWidth = math.fabs(experiment.field_height_um * c) + math.fabs(experiment.field_width_um * s)
        
        patch.setShaderVector( name='wavelength',
                             x = startWavelength / patternLength * 1.4142135623730950488016887242097,
                             y = endWavelength / patternLength * 1.4142135623730950488016887242097 )
        patch.setShaderVector( name='contrastLevels', x=minContrast, y=maxContrast )
        patch.setShaderVector( name='span', x=c/patternLength, y=s/patternLength )
        patch.setShaderVector( name='cspan', x=c/patternWidth, y=s/patternWidth )
        patch.setShaderVariable( name='sineExponent', value=sineExponent )
        patch.setShaderVariable( name='contrastGradientExponent', value=contrastGradientExponent )
        patch.setShaderFunction( name = patternFunctionName, src = '''
            vec3 {pattern}(vec3 shapeMask, vec2 x){{
                float t = dot(x, span) + 0.5;
                float s = clamp(cross(vec3(x, 0), vec3(cspan, 0)).z + 0.5, 0, 1);
               	float q = log(- wavelength.x * t + wavelength.x + wavelength.y * t) / (wavelength.x + wavelength.y);
        		q -= log(wavelength.x) / (wavelength.x + wavelength.y);
                if(abs(wavelength.x - wavelength.y) < 0.000001)
                        q = t / wavelength.x * 1.4142135623730950488016887242097;
        		float f = cos (q * 6.283185307179586476925286766559);
		        if(f>0)
			        f = pow(f, sineExponent);
		        else
			        f = -pow(-f, sineExponent);
                float ys;
		        if(contrastGradientExponent > 0)
			        ys = pow(s, contrastGradientExponent);
		        else
			        ys = pow(1-s, -contrastGradientExponent);
		        ys = ys * contrastLevels.y + (1-ys) * contrastLevels.x;
                return mix(background, mix(color1, color2, 0.5 - f * ys * 0.5), shapeMask);
            }}
        '''.format( pattern=patternFunctionName )  ) 