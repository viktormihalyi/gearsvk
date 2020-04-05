import Gears as gears
from .. import * 
import math
from .Base import *

class MouseCrossing(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            offset_um       : 'Signed distance of linear motion path from the origin (positive is to the left) [um].'
                            = 0,
            shapeLength_um  : 'The signed distance from the field edges where the motion starts and ends [um]. Positive is outside.'
                            = 2000,
            minSpeed        = -1200,
            maxSpeed        = 1200
            ) :
        sequence = spass.getSequence().getPythonObject()

        spass.setShaderVector( name=functionName + '_startPosition', x=0, y=0 )
        spass.setShaderVector( name=functionName + '_velocity', x=0, y=0 )
        spass.setShaderVector( name=functionName + '_radiusAndPhase', x=sequence.field_width_um * 0.5, y=0 )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            mat3x2 @<X>@ (float time){ return mat3x2(vec2(1, 0), vec2(0, 1), -`startPosition - `velocity * (mod(time + `radiusAndPhase.y, `radiusAndPhase.x * 2) - `radiusAndPhase.x) ); }
            ''' ).format( X = functionName ) )

        self.minSpeed = minSpeed
        self.maxSpeed = maxSpeed
        self.speedStep = 50
        self.angleStep = 0.1

        self.functionName = functionName
        self.spass = spass
        stimulus = spass.getStimulus().getPythonObject()
        #stimulus.onMouse += [ self.onMouse ]
        #stimulus.onMouseClick += [ self.onMouseClick ]
        stimulus.addTag("speed")
        stimulus.addTag("direction")
        stimulus.registerCallback(gears.WheelEvent.typeId, self.onWheel )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        stimulus.registerCallback(gears.KeyReleasedEvent.typeId, self.onKeyUp )
        stimulus.registerCallback(gears.FrameEvent.typeId, self.onFrame )

        #self.prevX = 0
        #self.prevY = 0
        self.speed = 0
        self.angle = 0
        self.facing = (1, 0)
        self.radius = sequence.field_width_um
        self.phase = 0
        self.shapeLength_um = shapeLength_um

        self.speedKeyDown = False
        self.angleKeyDown = False

        self.time = 0

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
    #            self.facing = (deltaX / l, -deltaY / l )
    #    if event.buttons() & Qt.RightButton :
    #        x = event.globalX()
    #        deltaX = x - self.prevX
    #        self.speed += deltaX

    def onWheel(self, event) :
        stimulus = self.spass.getStimulus()
        sequence = stimulus.getSequence().getPythonObject()
        if self.speedKeyDown :
            travelLength = math.fabs(sequence.field_width_um * self.facing[0]) + math.fabs(sequence.field_height_um * self.facing[1]) + self.shapeLength_um

            currentOffs = self.fmod(self.time + self.phase, self.radius * 2) - self.radius
            currentOffs *= self.speed

            self.speed += event.deltaY() * self.speedStep / 120
            if self.speed < self.minSpeed:
                self.speed = self.minSpeed
            if self.speed > self.maxSpeed:
                self.speed = self.maxSpeed

            if self.speed > 0.01 :
                self.radius = travelLength / self.speed / 2
                currentOffs /= self.speed
            elif self.speed < -0.01 :
                self.radius = travelLength / -self.speed / 2
                currentOffs /= self.speed

            self.phase = self.fmod((currentOffs + self.radius) - self.time, self.radius * 2)

            gears.setText("speed", "[V]+wheel:  speed: {speed} um/s in[{minSpeed},{maxSpeed}], step {step}".format(speed = self.speed, minSpeed=self.minSpeed, maxSpeed = self.maxSpeed, step=self.speedStep))
        elif self.angleKeyDown :
            self.angle += event.deltaY() * self.angleStep / 120
            self.facing = (math.cos(self.angle), math.sin(self.angle))
            gears.setText("direction", "[D]+wheel:  angle: {angle} radians, step {step}".format(angle = self.angle, step=self.angleStep))

    def onKey(self, event):
        if event.text() == 'O':
            self.speed = 0
        elif event.text() == ' ':
            gears.setText("speed", "[V]+wheel:  speed: {speed} um/s in[{minSpeed},{maxSpeed}], step {step}".format(speed = self.speed, minSpeed=self.minSpeed, maxSpeed = self.maxSpeed, step=self.speedStep))
            gears.setText("direction", "[D]+wheel:  angle: {angle} radians, step {step}".format(angle = self.angle, step=self.angleStep))
        elif event.text() == 'V':
            self.speedKeyDown = True
        elif event.text() == 'D':
            self.angleKeyDown = True
    
    def onKeyUp(self, event):
        if event.text() == 'O':
            self.speed = 0
        elif event.text() == 'V':
            self.speedKeyDown = False
        elif event.text() == 'D':
            self.angleKeyDown = False


    def fmod(self, x, y):
        return x - y * math.floor(x/y)

    def onFrame(self, event):
        self.time = event.time

        self.spass.setShaderVector( name=self.functionName+'_velocity', x=self.facing[0]*self.speed, y=self.facing[1]*self.speed )

        self.spass.setShaderVector( name=self.functionName+'_radiusAndPhase', x=self.radius, y=self.phase )



