import Gears as gears
from .. import * 

class Erf(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            toneRangeMean = 0.5,
            toneRangeVar = 0.3,
            dynamic = False
            ) :

        self.stimulus = stimulus;
        try:
            trmean = toneRangeMean.value
        except:
            trmean = toneRangeMean
        try:
            trvar = toneRangeVar.value
        except:
            trvar = toneRangeVar

        stimulus.setToneMappingErf(trmean, trvar, dynamic)
        self.registerInteractiveControls(
            None, stimulus, 
            "",
            toneRangeMean  = toneRangeMean ,
            toneRangeVar = toneRangeVar  ,
            )


    def update(self, **kwargs):
        for key, control in self.interactiveControls.items() :
            if key == 'toneRangeMean' :
                try:
                    self.stimulus.toneRangeMean = control.value
                except:
                    self.stimulus.toneRangeMean = control
            if key == 'toneRangeVar' :
                try:
                    self.stimulus.toneRangeVar = control.value
                except:
                    self.stimulus.toneRangeVar = control



