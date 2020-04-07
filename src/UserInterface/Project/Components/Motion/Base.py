import Gears as gears
from .. import * 

class Base(Component) : 
    def apply(self, spass, functionName) :
        if self.tb == None :
            self.tb = spass.tb
        self.applyWithArgs(spass, functionName, **self.args)

    def applyForward(self, spass, functionName) :
        if self.tb == None :
            self.tb = spass.tb
        self.applyForwardWithArgs(spass, functionName, **self.args)

