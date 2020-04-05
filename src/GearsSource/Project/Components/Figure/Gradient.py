import Gears as gears
from .. import * 
from .Base import *

class Gradient(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color1              : '"bright" pattern color.'
                                = 'white',
            color2              : '"dark" pattern color.'
                                = 'black',
            direction           : "The pattern direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')."
                                = 'east',
            start               : 'Distance from field center where gradient starts [um], or "edge" to start at the edge or corner.'
                                = 'edge',
            end                 : 'Distance from field center where gradient end [um], or "edge" to end at the edge or corner.'
                                = 'edge'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)

        stimulus = spass.getStimulus()

        if max(color1) - min(color1) > 0.03 or max(color2) - min(color2) > 0.03:
            stimulus.enableColorMode()

        spass.setShaderColor( name = functionName+'_color1', red = color1[0], green=color1[1], blue=color1[2] )
        spass.setShaderColor( name = functionName+'_color2', red = color2[0], green=color2[1], blue=color2[2] )

        direction = processDirection(direction, self.tb)
        sequence = stimulus.getSequence()
        s = math.sin(direction)
        c = math.cos(direction)
        patternLength = math.fabs(sequence.field_width_um * c) + math.fabs(sequence.field_height_um * s)
        if start == 'edge' :
            start = - patternLength / 2
        if end == 'edge' :
            end =  patternLength / 2
        spass.setShaderVector( name = functionName+'_span', x=c, y=s )
        spass.setShaderVector( name = functionName+'_gradientEnds', x = start, y = end )
        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@(vec2 x, float time){
                    float t = dot(x, @<X>@_span);
                    t = clamp((t - @<X>@_gradientEnds.x) / (@<X>@_gradientEnds.y - @<X>@_gradientEnds.x), 0, 1);
                    return mix(@<X>@_color1, @<X>@_color2, t); }
        ''').format( X=functionName )  ) 

