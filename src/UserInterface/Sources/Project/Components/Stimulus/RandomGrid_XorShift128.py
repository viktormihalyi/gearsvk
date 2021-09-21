import GearsModule as gears
from .. import * 
from .SingleShape import *

class RandomGrid_XorShift128(SingleShape) :

    def boot(self,
            *,
            duration: int = 1,
            duration_s: int  = 0,
            name: str = 'RandomGrid',
            randomSeed: int = 3773623027,
            randomGridSize: tuple[int, int] = (38, 38),
            color1: str = 'white',
            color2: str = 'black',
            spatialFilter = Spatial.Nop()):
        if name == 'RandomGrid' :
            name = '{a}x{b}'.format(a = randomGridSize[0], b = randomGridSize[1])
        super().boot(name=name, duration=duration, duration_s=duration_s,
                shape = Pif.RandomGrid(),
                pattern = Pif.Solid(color=color1,),
                background = Pif.Solid(color=color2,),
                prng =  Prng.XorShift128(randomGridSize = randomGridSize,),
                spatialFilter = spatialFilter,)


