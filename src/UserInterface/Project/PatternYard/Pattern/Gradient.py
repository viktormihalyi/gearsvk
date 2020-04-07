import Gears as gears
from .. import * 

class Gradient(Component) : 

    def applyWithArgs(
            self,
            patch,
            *,
            color1              : '"bright" pattern color.'
                                = 'white',
            color2              : '"dark" pattern color.'
                                = 'black',
            background          : 'Color outside of shape.'
                                = 'black',
            direction           : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')."
                                = 'east',
            start               : 'Distance from field center where gradient starts [um], or "edge" to start at the edge or corner.'
                                = 'edge',
            end                 : 'Distance from field center where gradient end [um], or "edge" to end at the edge or corner.'
                                = 'edge',
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
        if start == 'edge' :
            start = - patternLength / 2
        if end == 'edge' :
            end =  patternLength / 2
        patch.setShaderVector( name='span', x=c, y=s )

        patch.setShaderVector( name = 'gradientEnds', x = start, y = end )
        
        patch.setShaderFunction( name = patternFunctionName, src = '''
                vec3 {pattern}(vec3 shapeMask, vec2 x){{
                    float t = dot(x, span);
                    t = clamp((t - gradientEnds.x) / (gradientEnds.y - gradientEnds.x), 0, 1);
                    return mix(background, mix(color1, color2, t), shapeMask); }}
        '''.format( pattern=patternFunctionName )  ) 

