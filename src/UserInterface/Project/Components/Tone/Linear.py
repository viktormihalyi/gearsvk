import Gears as gears
from .. import * 

class Linear(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            toneRangeMin = 0.0,
            toneRangeMax = 1.0,
            dynamic = False
            ) :

        self.stimulus = stimulus;
        try:
            trmin = toneRangeMin.value
        except:
            trmin = toneRangeMin
        try:
            trmax = toneRangeMax.value
        except:
            trmax = toneRangeMax

        stimulus.setToneMappingLinear(trmin, trmax, dynamic)
        self.registerInteractiveControls(
            None, stimulus, 
            "",
            toneRangeMin  = toneRangeMin ,
            toneRangeMax = toneRangeMax  ,
            )


    def update(self, **kwargs):
        for key, control in self.interactiveControls.items() :
            if key == 'toneRangeMin' :
                try:
                    self.stimulus.toneRangeMin = control.value
                except:
                    self.stimulus.toneRangeMin = control
            if key == 'toneRangeMax' :
                try:
                    self.stimulus.toneRangeMax = control.value
                except:
                    self.stimulus.toneRangeMax = control



