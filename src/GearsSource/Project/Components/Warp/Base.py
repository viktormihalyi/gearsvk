import Gears as gears
from .. import * 
from .Base import *

class Base(Component) : 
    def apply(self, spass, functionName) :
        if self.tb == None :
            self.tb = spass.tb
        self.applyWithArgs(spass, functionName, **self.args)

