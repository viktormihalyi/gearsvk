import Gears as gears
from .. import * 
from .Base import *

class Crossing(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            velocity        : 'Motion speed [um/s].'
                            = 100,
            direction       : "The motion direction in radians (or 'east', 'northeast', 'north', 'northwest', 'west', 'southwest', 'south', or 'southeast')"
                            = 'east',
            offset_um       : 'Signed distance of linear motion path from the origin (positive is to the left) [um].'
                            = 0,
            shapeLength_um  : 'The signed distance from the field edges where the motion starts and ends [um]. Positive is outside.'
                            = 50,
            travelLength_um : 'The distance travelled by the shape. If zero, the distance is computed so that the shape crosses the entire screen.'
                            = 0,
            extendStimulusDurationToCrossingDuration : 'If True, and stimulus duration is less than the crossing duration, it is set to be equal.'
                            = True
            ) :
        sequence = spass.getSequence().getPythonObject()
        stimulus = spass.getStimulus().getPythonObject()
        direction = processDirection(direction, self.tb)
        s = math.sin(direction)
        c = math.cos(direction)
        if travelLength_um == 0:
            travelLength_um = math.fabs(sequence.field_width_um * c) + math.fabs(sequence.field_height_um * s) + shapeLength_um
        spass.duration = int(travelLength_um / velocity / sequence.getFrameInterval_s() + 1.0 )
        if extendStimulusDurationToCrossingDuration:
            stimulus.duration = max(stimulus.duration, spass.duration)

        spass.setShaderVector( name = functionName + '_startPosition', x=-travelLength_um*0.5*c - offset_um*s, y=-travelLength_um*0.5*s + offset_um*c )
        spass.setShaderVector( name = functionName + '_velocity', x=velocity*c, y=velocity*s )

        spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
            mat3x2 @<X>@ (float time){ return mat3x2(vec2(1, 0), vec2(0, 1), -`startPosition - `velocity * time); }
            ''' ).format( X = functionName ) )


