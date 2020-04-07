import Gears as gears
from .. import * 

class Base(Component) : 

    def apply(self, spass, functionName) :
        if self.tb == None :
            self.tb = spass.tb
        self.applyWithArgs(spass, functionName, **self.args)

    def __lshift__(self, other):
        from .. import Composition 
        if isinstance(other, Modulation.Base) :
            return Composition.Modulate(pif=self, modulation=other)
        elif isinstance(other, Warp.Base) :
            return Composition.Warp(pif=self, warp=other)
        elif isinstance(other, Motion.Base) :
            return Composition.Move(pif=self, motion=other)
        elif isinstance(other, TimeWarp.Base) :
            return Composition.TimeWarp(pif=self, timeWarp=other)

    def __pow__(self, other):
        from .. import Composition
        if isinstance(other, Base) :
            return Composition.Mix(shape = self, foregroundPattern = other)
        elif isinstance(other, tuple) :
            return Composition.Mix(shape = self, foregroundPattern = other[0], backgroundPattern = other[1])

    def __add__(self, other):
        from .. import Composition
        if isinstance(other, Base) :
            return Composition.Add(pif1= self, pif2 = other)

    def __sub__(self, other):
        from .. import Composition
        if isinstance(other, Base) :
            return Composition.Sub(pif1= self, pif2 = other)

    def __mul__(self, other):
        from .. import Composition
        if isinstance(other, Base) :
            return Composition.Mul(pif1= self, pif2 = other)
