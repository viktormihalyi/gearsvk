import Gears as gears
from .. import * 
import math
import ILog
from .Control import *

class MouseWheel(Control): 

    def __init__(
            self,
            *,
            initialValue        : 'Initial value.'
                                = 1,
            minimum             : 'Minimum.'
                                = 0,
            maximum             : 'Maximum.'
                                = 1,
            step                : 'Step size'
                                = 0,
            key                 : 'Mouse wheel control is active when this key is held down.'
                                = 'X',
            label               : 'Text displayed on-screen. Must be unique, or only the last item of the same name is displayed.'
                                = 'Interactive setting'
            ) :
        super().__init__()
        self.isDirection = False

        self.step = step
        self.maximum = maximum
        self.minimum = minimum
        if self.step == 0 :
            self.step = (self.maximum - self.minimum) / 100
        self.keyDown = False
        self.label = label
        self.key = key
        self.unit = 'um'

        self.keyDown = False

        #try:
        #    self.angle = processDirection(initialValue, None)
        #    self.value = (math.cos(self.angle), math.sin(self.angle))
        #    gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.angle, unit=self.unit, step=self.step))
        #except:
        self.value = initialValue
        gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.value, unit=self.unit, step=self.step))

    def setDirection(self):
        self.isDirection = True
        self.angle = processDirection(self.value, None)
        self.value = (math.cos(self.angle), math.sin(self.angle))
        gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.angle, unit=self.unit, step=self.step))


    def onWheel(self, event, owner) :
        if self.keyDown:
            if self.isDirection:
                self.angle += event.deltaY() * self.step / 120
                if self.angle > self.maximum:
                    self.angle = self.maximum
                if self.angle < self.minimum:
                    self.angle = self.minimum
                self.value = (math.cos(self.angle),  math.sin(self.angle))
                gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.angle, unit=self.unit, step=self.step))
            else:
                self.value += event.deltaY() * self.step / 120
                if self.value > self.maximum:
                    self.value = self.maximum
                if self.value < self.minimum:
                    self.value = self.minimum
                gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.value, unit=self.unit, step=self.step))
            owner.refresh()

    def onKey(self, event):
        if event.text() == self.key:
            self.keyDown = True
        elif event.text() == ' ':
            if self.isDirection:
                gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.angle, unit=self.unit, step=self.step))
            else:
                gears.setText(self.label, "[{key}]+wheel: {label}: {value} {unit}, step {step} {unit}".format(key = self.key, label=self.label, value=self.value, unit=self.unit, step=self.step))
    
    def onKeyUp(self, event):
        if event.text() == self.key:
            self.keyDown = False

    #def applyAsShaderParameter(self, owner, varname):
    #    if not self.isDirection:
    #        owner.setShaderVariable(name=varname, value=self.value)
    #    else:
    #        owner.setShaderVector(name=varname, x = math.cos(self.value), y = math.sin(self.value))
    #    ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), var=self.label, value=self.value))

    def register(self, stimulus, component):
        stimulus.addTag(self.label)
        stimulus.registerCallback(gears.WheelEvent.typeId, lambda event: self.onWheel(event, component) )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        stimulus.registerCallback(gears.KeyReleasedEvent.typeId, self.onKeyUp )





