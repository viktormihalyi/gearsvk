import Gears as gears
from .. import * 
from .Base import *

class Mobius(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            a  : 'a'
                    = (1, 0),
            b  : 'b'
                    = (0, 0),
            c  : 'c'
                    = (0, 0),
            d  : 'd'
                    = (1, 0)
            ) :

        sequence = spass.getSequence()
        stimulus = spass.getStimulus()

        self.registerInteractiveControls(
                spass, stimulus,
                functionName+'_',
                a    =   a,
                b    =   b,
                c    =   c,
                d    =   d,
                )

        spass.setShaderFunction(name = "cmul", src = self.glslEsc( '''
                vec2 cmul(vec2 a, vec2 b){return vec2(a.x * b.x - a.y * b.y, a.x*b.y + a.y*b.x); }
        ''') )
        spass.setShaderFunction(name = "cinv", src = self.glslEsc( '''
                vec2 cinv(vec2 a){return a*vec2(1, -1)/dot(a,a); }
        ''') )
        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                vec2 @<X>@(vec2 x){return 
                cmul(
                  cmul(x, `a) + `b,
                  cinv(
                    cmul(x, `c) + `d
                    )
                  );
                    }
        ''').format( X=functionName )  ) 
