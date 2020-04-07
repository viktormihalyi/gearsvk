import Gears as gears
from .. import * 
from .SingleShape import *

class Spot(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Spot',
            radius          : 'Disc radius [um].'
                            =   150,
            innerRadius     : 'Annulus hole radius [um]. Zero for solid disc.'
                            =   0,
            position        : 'Spot center position as an (x,y) pair [(um, um)]. (0,0) is the field center.'
                            =   (0, 0),
            color           : 'Spot color as an (r,g,b) triplet or a color name.'
                            =   'white',
            background      : 'Background color as an (r,g,b) triplet or a color name.'
                            =   'black',
            spatialFilter   : 'Spatial filter component. (Spatial.*)'
                            = Spatial.Nop(),
            temporalFilter  : 'Temporal filter component. (Temporal.*)'
                            = Temporal.Nop(),
            modulation      : "Intensity modulation component. (Modulation.*)"
                            = Modulation.Linear(),
            motion          : "Motion component. (Motion.*) If specified, parameter 'position' is ignored."
                            = None

            ):
        if name == 'Spot' and type(position[0]) == str :
            name = '{r}{c}'.format( r=position[0], c=position[1])
        elif name == 'Spot' :
            name = 'Spot {r} um'.format(r = radius)
        if motion == None :
            motion = Motion.Linear( 
                            startPosition=position,
                            )
        #if spatialFilter == None :
        #    spatialFilter = Spatial.Nop()
        #if temporalFilter == None :
        #    temporalFilter = Temporal.Nop()
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     modulation =   modulation,
                     shape = Pif.Spot( 
                            radius  =   radius,
                            innerRadius =   innerRadius,
                            ),
                     pattern = Pif.Solid(
                            color = color,
                            ),
                     background = Pif.Solid(
                            color = background,
                            ),
                     shapeMotion = motion,
                     gamma = Gamma.Linear(),
                     spatialFilter = spatialFilter,
                     temporalFilter = temporalFilter,
                     #rasterizingMode = 'quads',
                     #polygonMask = [{'x':0, 'y':0, 'width':radius*2, 'height':radius*2, 'pif':0}],
                     )