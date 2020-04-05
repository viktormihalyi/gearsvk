import Gears as gears
from .. import * 
from .Base import *

class BinaryWithTemporalWeighting(Base) : 

    def applyWithArgs(
            self,
            spass,
            *,
            brightColor : 'Modulating color at unit modulated intensity.'
                        = 'white',
            darkColor   : 'Modulating color at zero modulated intensity.'
                        = 'black',
            switches    : 'List of time points when intensity is toggled [s]. Negative values are measured back from duration end.'
                        = [2, -2],
            weighting   : 'Temporal filter used for intensity filtering.'
                        = Temporal.Nop()
            ) :
        duration = spass.getDuration_s()
        self.switches = [t if t>0 else duration+t for t in switches]

        self.brightColor = processColor(brightColor, self.tb)
        self.darkColor = processColor(darkColor, self.tb)
        self.stimulus = self.getStimulus()
        self.weights = weighting.getTemporalWeights(self.stimulus)
        self.weights.reverse()
        print(self.weights)
                                 
        spass.setShaderColor( name = 'globalIntensity', red=0, green=0, blue=0 )

        spass.setShaderFunction( name = 'intensity', src = 'vec3 intensity(float time){ return globalIntensity; }'  )

        self.stimulus.registerCallback(gears.StimulusFrame(), self.onFrame )
        self.stimulus.setTemporalWeights( weighting.getTemporalWeights(self.stimulus), False )

    def onFrame(self, time):
        frameInterval_s = self.getSequence().getFrameInterval_s()
        iSwitch = 0
        bright = False
        l = 0
        if time > 2.15:
            pass
        time -= frameInterval_s * (len(self.weights) - 1)
        for u in self.weights:
            try :
                while self.switches[iSwitch] < time :
                    iSwitch += 1
                    bright = not bright
            except IndexError:
                pass
            if bright :
                l += u
            time += frameInterval_s
        #print(l*1+(1-l)*0)
        #l = math.sin(time * 10) * 0.5 + 0.5
        self.setShaderColor( name = 'globalIntensity', 
                red=    l*self.brightColor[0] + (1-l)*self.darkColor[0],
                green=  l*self.brightColor[1] + (1-l)*self.darkColor[1],
                blue=   l*self.brightColor[2] + (1-l)*self.darkColor[2] )

