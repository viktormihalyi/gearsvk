import Gears as gears
from .. import * 

class Base(Component) : 
    def apply(self, spass, functionName) :
        if self.tb == None :
            self.tb = spass.tb
        self.applyWithArgs(spass, functionName, **self.args)


    def __mul__(self, other):
        from .Product import Product
        return Product(op0 = self, op1 = other)

    def __sub__(self, name):
        self.args['displayName'] = name
        return self

