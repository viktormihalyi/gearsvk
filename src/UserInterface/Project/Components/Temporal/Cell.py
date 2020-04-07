import Gears as gears
from .. import * 
from .Filter import *
import math

class Cell(Filter) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            gtp1    : 'Gaussian term 1 mean [frame].'
                    =	2.0	     ,
            gtp2    : 'Gaussian term 2 mean [frame].'
                    =	5.0      ,
            ga1     : 'Gaussian term 1 weight.'
                    =	0.8		 ,
            ge1     : 'Gaussian term 1 variance [frame^2]'
                    =	0.06	 ,
            ga2     : 'Gaussian term 2 weight'
                    =	0.26	 ,
            ge2     : 'Gaussian term 2 variance [frame^2]'
                    =	0.024	 ,
            gcut    : 'Clipping exponent.'
                    =	0.2	     
            ) :
        self.ga1 = ga1
        self.ga2 = ga2
        self.ge1 = ge1
        self.ge2 = ge2
        self.gtp1 = gtp1
        self.gtp2 = gtp2
        self.gcut = gcut

        sequence = stimulus.getSequence().getPythonObject()

        stimulus.setTemporalWeights( self.getTemporalWeights(stimulus), False )

        stimulus.setTemporalWeightingFunction(
            """
                float temporalWeight(int t)
                {{
                    return
                    (
                    {ga1} * exp(-{ge1} * (t-{gtp1}) * (t-{gtp1})) 
                    -
                    {ga2} * exp(-{ge2} * (t-{gtp2}) * (t-{gtp2}))
                    ) 
                    * ( 1-exp(-{gcut} * t) );
                }}
            """.format(ga1=ga1, ge1=ge1, ga2=ga2, ge2=ge2, gtp1=gtp1, gtp2=gtp2, gcut=gcut),
            32,
            True,
            min(self.getTemporalWeights(stimulus) ),
            max(self.getTemporalWeights(stimulus) )
            )

    def getTemporalWeightsWithArgs(
            self,
            stimulus,
            *,
            gtp1    : 'Gaussian term 1 mean [frame].'
                    =	2.0	     ,
            gtp2    : 'Gaussian term 2 mean [frame].'
                    =	5.0      ,
            ga1     : 'Gaussian term 1 weight.'
                    =	0.8		 ,
            ge1     : 'Gaussian term 1 variance [frame^2]'
                    =	0.06	 ,
            ga2     : 'Gaussian term 2 weight'
                    =	0.26	 ,
            ge2     : 'Gaussian term 2 variance [frame^2]'
                    =	0.024	 ,
            gcut    : 'Clipping exponent.'
                    =	0.2	  ) :
        weights = []
        for t in range(0, 64):
            weights += [(ga1 * math.exp(-ge1 * (t-gtp1) * (t-gtp1))
                    -
                    ga2 * math.exp(-ge2 * (t-gtp2) * (t-gtp2))
                    ) * ( 1-math.exp(-gcut * t) )]
        return weights
    