import Gears as gears
from .. import * 
import math
from .Base import *

class MouseRect(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            size_um             : 'Dimensions of the rectangle shape, as a (width, height) pair [um,um].'
                                = (2000, 200),
            facing              : "The shape orientation in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')"
                                = 'east',
            follow_distance_um  : 'Distance between instances of the rectangle along the width axis [um].'
                                = 100000000,
            wingmen_distance_um : 'Distance between instances of the rectangle along the height axis [um].'
                                = 100000000,
            filterRadius_um     : 'Antialiasing filter size [um] (shape blur).'
                                = 0.1
            ) :
        facing = processDirection(facing, self.tb)
        s = math.sin(facing)
        c = math.cos(facing)

        spass.setShaderVector( name = functionName+'_rect',    x = size_um[0], y= size_um[1] )
        spass.setShaderVector( name=functionName+'_facing', x=c, y=s )
        spass.setShaderVector( name=functionName+'_repetitionDistance', x=follow_distance_um, y=wingmen_distance_um )
        spass.setShaderVariable( name=functionName+'_filterRadius', value = filterRadius_um )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec3 @<X>@ (vec2 x, float time){ 
                    vec2 rotatedX = vec2( x.x * `facing.x + x.y * `facing.y, x.x * `facing.y - x.y * `facing.x);
                    rotatedX = mod(rotatedX + `repetitionDistance*0.5 , `repetitionDistance) - `repetitionDistance * 0.5;
                    float xDiff = abs(rotatedX.x) - `rect.x * 0.5;
                    float yDiff = abs(rotatedX.y) - `rect.y * 0.5;
                    float inOrOut = (1-smoothstep( -`filterRadius, +`filterRadius, xDiff )) * (1-smoothstep( -`filterRadius, +`filterRadius, yDiff ));
                    return vec3(inOrOut, inOrOut, inOrOut);
                    }
        ''').format( X=functionName )  ) 

        self.functionName = functionName
        self.spass = spass
        stimulus = spass.getStimulus().getPythonObject()
        stimulus.registerCallback(gears.WheelEvent.typeId, self.onWheel )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        stimulus.registerCallback(gears.KeyReleasedEvent.typeId, self.onKeyUp )
        stimulus.registerCallback(gears.FrameEvent.typeId, self.onFrame )

        self.angleStep = 0.1
        self.angle = 0
        self.facing = (1, 0)
        self.angleKeyDown = False

        gears.setText("direction", "[D]+wheel:  angle: {angle} radians, step {step}".format(angle = self.angle, step=self.angleStep))

    def onWheel(self, event) :
        sequence = self.spass.getSequence().getPythonObject()
        if self.angleKeyDown :
            self.angle += event.deltaY() * self.angleStep / 120
            self.facing = (math.cos(self.angle), math.sin(self.angle))
            gears.setText("direction", "[D]+wheel:  angle: {angle} radians, step {step}".format(angle = self.angle, step=self.angleStep))

    def onKey(self, event):
        if event.text() == 'D':
            self.angleKeyDown = True
        elif event.text() == ' ':
            gears.setText("direction", "[D]+wheel:  angle: {angle} radians, step {step}".format(angle = self.angle, step=self.angleStep))
    
    def onKeyUp(self, event):
        if event.text() == 'D':
            self.angleKeyDown = False

    def onFrame(self, event):
        self.time = event.time
        
        self.spass.setShaderVector( name=self.functionName+'_facing', x=self.facing[0], y=self.facing[1] )



