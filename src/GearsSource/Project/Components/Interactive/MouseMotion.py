import Gears as gears
from .. import * 
import math
from .Control import *
import ILog

class MouseMotion(Control): 

    def __init__(
            self,
            *,
            initialValue        : 'Initial value.'
                                = (100, 100),
            minimum             : 'Minimum.'
                                = (0, 0),
            maximum             : 'Maximum.'
                                = (1000, 1000),
            key                 : 'Mouse move control is active when this key is held down.'
                                = 'X',
            label               : 'Text displayed on-screen. Must be unique, or only the last item of the same name is displayed.'
                                = 'Interactive setting'
            ) :

        super().__init__()
        self.value = initialValue
        self.maximum = maximum
        self.minimum = minimum
        self.keyDown = False
        self.label = label
        self.key = key
        self.unit = 'um'

        self.keyDown = False

        gears.setText(self.label, "[{key}]+mouse: {label}: {x} {unit}, {y} {unit}".format(key = self.key, label=self.label, x=self.value[0], y=self.value[1], unit=self.unit))

    def onMouseMove(self, event, owner) :
        if self.keyDown:
            self.value = (event.globalPercentX() * (self.maximum[0] - self.minimum[0]) + self.minimum[0],
                        event.globalPercentY() * (self.maximum[1] - self.minimum[1]) + self.minimum[1])
            owner.refresh()
            gears.setText(self.label, "[{key}]+mouse: {label}: {x} {unit}, {y} {unit}".format(key = self.key, label=self.label, x=self.value[0], y=self.value[1], unit=self.unit))

    def onKey(self, event):
        if event.text() == self.key:
            if not self.keyDown:
                pass
                gears.setMousePointerLocation(
                    (self.value[0] - self.minimum[0]) / (self.maximum[0] - self.minimum[0]), 
                    (self.value[1] - self.minimum[1]) / (self.maximum[1] - self.minimum[1]))
            self.keyDown = True
        elif event.text() == ' ':
           gears.setText(self.label, "[{key}]+mouse: {label}: {x} {unit}, {y} {unit}".format(key = self.key, label=self.label, x=self.value[0], y=self.value[1], unit=self.unit))

    
    def onKeyUp(self, event):
        if event.text() == self.key:
            self.keyDown = False

    #def applyAsShaderParameter(self, owner, varname):
    #    owner.setShaderVector(name=varname, x=self.value[0], y=self.value[1])
    #    #logging is in Component.refresh now
    #    ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), var=self.label, value=self.value))

    def register(self, stimulus, component):
        stimulus.addTag(self.label)
        stimulus.registerCallback(gears.MouseMoveEvent.typeId, lambda event: self.onMouseMove(event, component) )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        stimulus.registerCallback(gears.KeyReleasedEvent.typeId, self.onKeyUp )





