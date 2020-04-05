import Gears as gears
from .. import * 
from .SingleShape import *

class CrossingRect(SingleShape) :

    def boot(self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'CrossingRect',
            size_um         : 'Dimensions of the rectangle in um.'
                            = (50, 200),
            color           : 'Rectangle color as rgb triplet or color name.'
                            = 'white',
            direction       : "The direction of motion in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')."
                            = 'east',
            velocity        : 'The speed of motion [um/s].'
                            = 1200,
            offset_um       : 'The distance of the motion path from the center [um].'
                            = 0,
            travelLength_um : 'The distance travelled by the shape. If zero, the distance is computed so that the shape crosses the entire screen.'
                            = 0,
            follow_distance_um  : 'Distance between two rectangles following each other [um].'
                                = 100000000,
            wingmen_distance_um : 'Distance between two rectangles moving in parallel [um].'
                                = 100000000,
            filterRadius_um     : 'Extents of intensity dropoff at rectangle edges [um]. E.g. for antialiasing.'
                                = 0.1,
            temporalFilter      : 'Temporal filtering object.'
                                = Temporal.Nop()
            ):
        if name == 'CrossingRect' :
            if velocity != 0 :
                name = str(direction)
            else :
                name = 'halt'
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     modulation =   Modulation.Linear( 
                            brightColor=color,
                            ),
                     shape = Pif.Rect( 
                            size_um     =   size_um,
                            facing      =   direction,
                            follow_distance_um = follow_distance_um,
                            wingmen_distance_um = wingmen_distance_um,
                            filterRadius_um = filterRadius_um,
                            ),
                     shapeMotion = Motion.Crossing(
                            direction       =   direction,
                            velocity        =   velocity,
                            shapeLength_um  =   size_um[0],
                            offset_um       =   offset_um,
                            travelLength_um = travelLength_um,
                            ),
                     temporalFilter = temporalFilter,
                     )