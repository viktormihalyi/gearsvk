import Gears as gears
from .. import * 
from .SingleShape import *

class SpotGrid(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'SpotGrid',
            radius          : 'Disc radius [um].'
                            =   0,
            innerRadius     : 'Annulus hole radius [um]. Zero for solid disc.'
                            =   0,
            position        : 'Reference spot center position as an (x,y) pair [(um, um)]. (0,0) is the field center.'
                            =   (0, 0),
            color           : 'Spot color as an (r,g,b) triplet or a color name.'
                            = 'white'
            ):
        sequence = self.getSequence().getPythonObject()
        if radius == 0 :
            radius = sequence.electrodeDistance_um[0] * 0.3
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     modulation =   Modulation.Linear( 
                            brightColor=color,
                            ),
                     shape = Pif.Spot( 
                            radius  =   radius,
                            innerRadius =   innerRadius,
                            ),
                     shapeMotion = Motion.Linear(
                            startPosition    = position,
                            ),
                     warp = Warp.OnElectrodes(
                            ),
                     )