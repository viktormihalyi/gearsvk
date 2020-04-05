import Gears as gears
from .. import * 
import math
from PyQt5.QtCore import Qt

class MouseRect(Component) : 

    def applyWithArgs(
            self,
            patch,
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

        patch.setShaderVector( name = 'rect',    x = size_um[0], y= size_um[1] )
        patch.setShaderVector( name='facing', x=c, y=s )
        patch.setShaderVector( name='repetitionDistance', x=follow_distance_um, y=wingmen_distance_um )
        patch.setShaderVector( name='rect', x=size_um[0], y=size_um[1] )
        patch.setShaderVariable( name='filterRadius', value = filterRadius_um )
        patch.setShaderFunction( name = 'shape', src = '''
                vec3 shape(vec2 x){ 
                    vec2 rotatedX = vec2( x.x * facing.x + x.y * facing.y, x.x * facing.y - x.y * facing.x);
                    rotatedX = mod(rotatedX + repetitionDistance*0.5 , repetitionDistance) - repetitionDistance * 0.5;
                    float xDiff = abs(rotatedX.x) - rect.x * 0.5;
                    float yDiff = abs(rotatedX.y) - rect.y * 0.5;
                    float inOrOut = (1-smoothstep( -filterRadius, +filterRadius, xDiff )) * (1-smoothstep( -filterRadius, +filterRadius, yDiff ));
                    return vec3(inOrOut, inOrOut, inOrOut);
                    }
        '''  )

        self.patch = patch
        stimulus = patch.getStimulus().getPythonObject()
        #stimulus.onMouse += [ self.onMouse ]
        #stimulus.onMouseClick += [ self.onMouseClick ]
        #self.prevX = 0
        #self.prevY = 0
        stimulus.registerCallback(gears.WheelEvent.typeId, self.onWheel )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        stimulus.registerCallback(gears.KeyReleasedEvent.typeId, self.onKeyUp )
        stimulus.registerCallback(gears.FrameEvent.typeId, self.onFrame )

        self.angleStep = 0.1
        self.angle = 0
        self.facing = (1, 0)
        self.angleKeyDown = False

        gears.setText("direction", "[D]+wheel:  angle: {angle} radians, step {step}".format(angle = self.angle, step=self.angleStep))

    #def onMouseClick(self, event) :
    #    self.prevX = event.globalX()
    #    self.prevY = event.globalY()
    #
    #def onMouse(self, event) :
    #    if event.buttons() & Qt.LeftButton :
    #        x = event.globalX()
    #        y = event.globalY()
    #        deltaX = x - self.prevX
    #        deltaY = y - self.prevY
    #        l = math.sqrt(deltaX * deltaX + deltaY * deltaY)
    #        if( l > 0.001):
    #            self.stimulus.setShaderVector( name='facing', x=deltaX / l, y=-deltaY / l )

    def onWheel(self, event) :
        experiment = self.patch.getExperiment().getPythonObject()
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
        #compute 
        #experiment = self.stimulus.getExperiment().getPythonObject()
        #travelLength = math.fabs(experiment.field_width_um * self.facing[0]) + math.fabs(experiment.field_height_um * self.facing[1]) + self.shapeLength_um
        #
        #currentOffs = self.fmod(time + self.phase, self.radius * 2) - self.radius
        #
        #if self.speed > 0.01 :
        #    self.radius = travelLength / self.speed
        #elif self.speed < -0.01 :
        #    self.radius = travelLength / -self.speed
        #self.phase = self.fmod((currentOffs + self.radius) - time, self.radius * 2)

        self.patch.setShaderVector( name='facing', x=self.facing[0], y=self.facing[1] )



