import Gears as gears
from .. import * 
from .SingleShape import *

class ScanRect(SingleShape) :

    def boot(
            self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'ScanRect',
            grid            : 'Dimensions of the grid on which the rectangle appears as a (columns, rows) pair.'
                            =   (7, 7),
            index           : 'The index of the grid cell that is highlighted, as a (column, row) pair.'
                            =   (0, 0),
            color           : 'Shape color as an (r,g,b) triplet or color name.'
                            =   'white'
            ):
        sequence = self.getSequence().getPythonObject()
        super().boot(name=name, duration=duration, duration_s=duration_s,
                     modulation =   Modulation.Linear( 
                            brightColor=color,
                            ),
                     shape = Pif.Rect( 
                            size_um  =   ( sequence.field_width_um / grid[0] , sequence.field_height_um / grid[1] ),
                            ),
                     shapeMotion = Motion.Linear(
                            startPosition   = ( ((index[0] + 0.5) / grid[0] - 0.5) * sequence.field_width_um, ((index[1] + 0.5) / grid[1] - 0.5) * sequence.field_height_um ),
                            ),
                     )