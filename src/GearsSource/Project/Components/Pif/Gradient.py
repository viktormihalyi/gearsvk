import Gears as gears
from .. import * 
from .Base import *

class Gradient(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            color1              : '"bright" pattern color, or Interactive.*'
                                = 'white',
            color2              : '"dark" pattern color, or Interactive.*'
                                = 'black',
            direction           : "<BR>The pattern direction in radians, <\BR><BR> or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast'<\BR><BR> or Interactive.*<\BR>"
                                = 'east',
            start               : 'Distance from field center where gradient starts [um], or "edge" to start at the edge or corner.'
                                = 'edge',
            end                 : 'Distance from field center where gradient end [um], or "edge" to end at the edge or corner.'
                                = 'edge'
            ) :
        color1 = processColor(color1, self.tb)
        color2 = processColor(color2, self.tb)

        stimulus = spass.getStimulus()
        sequence = stimulus.getSequence()

        if not isGrey(color1) or not isGrey(color2):
            spass.enableColorMode()

        #spass.setShaderColor( name = functionName+'_color1', red = color1[0], green=color1[1], blue=color1[2] )
        #spass.setShaderColor( name = functionName+'_color2', red = color2[0], green=color2[1], blue=color2[2] )
       
        direction = processDirection(direction, self.tb)
        try:
            direction.setDirection()
            s = math.sin(direction.value)
            c = math.cos(direction.value)
           
        except:
            s = math.sin(direction)
            c = math.cos(direction)
            direction = (c,s)
       
        patternLength = math.fabs(sequence.field_width_um * c) + math.fabs(sequence.field_height_um * s)
        try:
            start = start.value
        except:
            pass
        try:
            end = end.value
        except:
            pass
        if start == 'edge' :
            start = - patternLength / 2
        if end == 'edge' :
            end =  patternLength / 2
 
        
        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                span = direction,
                color1 = color1,
                color2 = color2,
                gradientEnds = (start, end)
                )
        #spass.setShaderVector( name = functionName+'_span', x=c, y=s )
        #spass.setShaderVector( name = functionName+'_gradientEnds', x = start, y = end )
        
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@(vec2 x, float time){
                    float t = dot(x, @<X>@_span);
                    t = clamp((t - @<X>@_gradientEnds.x) / (@<X>@_gradientEnds.y - @<X>@_gradientEnds.x), 0, 1);
                    return mix(@<X>@_color1, @<X>@_color2, t); }
        ''').format( X=functionName )  )


        def update(
            self,
            *,
            span = (1,0),
            color1 = 'white',
            color2 = 'black',
            gradientEnds = ('edge','edge')
            ):

            try:                
                s = math.sin(span.value)
                c = math.cos(span.value)
            except:
                s = span[1]
                c = span[0]
            try:
                start = gradientEnds[0].value
            except:
                start = gradientEnds[0]
            try:
                end = gradientEnds[1].value
            except:
                end =gradientEnds[1]
            patternLength = math.fabs(sequence.field_width_um * c) + math.fabs(sequence.field_height_um * s)
            if start == 'edge' :
                start = - patternLength / 2
            if end == 'edge' :
                end =  patternLength / 2
            #spass.setShaderVector( name = functionName+'_gradientEnds', x = start, y = end )
            Component.update(self,
                             span = span,
                             color1 = color1,
                             color2 = color2,
                             gradientEnds = (start, end),
                             )
            

   