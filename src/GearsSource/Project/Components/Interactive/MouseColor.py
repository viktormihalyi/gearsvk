import Gears as gears
from .. import * 
import math
import colorsys
from .Control import *
import ILog

class MouseColor(Control): 

    def __init__(
            self,
            *,
            initialValue        : 'Initial value.'
                                = 'white',
            key                 : 'Color control is active when this key is held down.'
                                = 'C',
            label               : 'Text displayed on-screen. Must be unique, or only the last item of the same name is displayed.'
                                = 'Interactive setting',
            sensitivity         :"Matrix of sensitivities (ch X 3)"
                                =0
            ) :
        super().__init__()

        self.value = processColor(initialValue, None)
        self.keyDown = False
        self.label = label
        self.key = key
        self.sensitivity = sensitivity
        
        if self.sensitivity !=0:
            self.s=[]
            for i in range(len(self.sensitivity)):
                self.s.append(sum([a*b for a,b in zip(self.sensitivity[i],self.value)]))
        self.keyDown = False

       
                   
    def onMouseMove(self, event, owner) :
        if self.keyDown:
            hls = colorsys.rgb_to_hls(*self.value)
            self.value = colorsys.hls_to_rgb(event.globalPercentX(), hls[1], event.globalPercentY())
            owner.refresh()

        if self.sensitivity !=0:
            for i in range(len(self.sensitivity)):
                self.s[i]=sum([a*b for a,b in zip(self.sensitivity[i],self.value)])  
            gears.setText(self.label, "[{key}]+mouse/wheel: {label}: {r:0.2f}, {g:0.2f}, {b:0.2f}\n".format(key = self.key, label=self.label, r=self.value[0], g=self.value[1], b=self.value[2])+"Sensitivities:"+', '.join(["%0.2f" % s for s in self.s]))
        else:
            gears.setText(self.label, "[{key}]+mouse/wheel: {label}: {r:0.2f}, {g:0.2f}, {b:0.2f}".format(key = self.key, label=self.label, r=self.value[0], g=self.value[1], b=self.value[2]))

    def onWheel(self, event, owner) :
        if self.keyDown:
            hls = colorsys.rgb_to_hls(*self.value)
            self.value = colorsys.hls_to_rgb(hls[0], hls[1] + event.deltaY() * 0.01 / 120, hls[2])
            owner.refresh()
        
        
        if self.sensitivity !=0:
            for i in range(len(self.sensitivity)):
                self.s[i]=sum([a*b for a,b in zip(self.sensitivity[i],self.value)])
            gears.setText(self.label, "[{key}]+mouse/wheel: {label}: {r:0.2f}, {g:0.2f}, {b:0.2f}\n".format(key = self.key, label=self.label, r=self.value[0], g=self.value[1], b=self.value[2])+"Sensitivities:"+', '.join(["%0.2f" % s for s in self.s]))
        else:
            gears.setText(self.label, "[{key}]+mouse/wheel: {label}: {r:0.2f}, {g:0.2f}, {b:0.2f}".format(key = self.key, label=self.label, r=self.value[0], g=self.value[1], b=self.value[2]))

    def onKey(self, event):
        if event.text() == self.key:
            if not self.keyDown:
                hls = colorsys.rgb_to_hls(*self.value)
                gears.setMousePointerLocation(hls[0], hls[2])
            self.keyDown = True
        elif event.text() == ' ':
            if self.sensitivity !=0:
                gears.setText(self.label, "[{key}]+mouse/wheel: {label}: {r:0.2f}, {g:0.2f}, {b:0.2f}\n".format(key = self.key, label=self.label, r=self.value[0], g=self.value[1], b=self.value[2])+"Sensitivities:"+', '.join(["%0.2f" % s for s in self.s]))
            else:
                gears.setText(self.label, "[{key}]+mouse/wheel: {label}: {r:0.2f}, {g:0.2f}, {b:0.2f}".format(key = self.key, label=self.label, r=self.value[0], g=self.value[1], b=self.value[2]))


    
    def onKeyUp(self, event):
        if event.text() == self.key:
            self.keyDown = False

    # def applyAsShaderParameter(self, owner, varname):
    #     owner.setShaderColor(name=varname, red=self.value[0], green=self.value[1], blue=self.value[2])
    #     ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), var=self.label, value=self.value))

    def register(self, stimulus, component):
        stimulus.addTag(self.label)
        stimulus.registerCallback(gears.WheelEvent.typeId, lambda event: self.onWheel(event, component) )
        stimulus.registerCallback(gears.MouseMoveEvent.typeId, lambda event: self.onMouseMove(event, component) )
        stimulus.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )
        stimulus.registerCallback(gears.KeyReleasedEvent.typeId, self.onKeyUp )

