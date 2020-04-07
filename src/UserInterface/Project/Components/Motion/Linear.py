import Gears as gears
from .. import * 
from .Base import *

class Linear(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            velocity        : 'Motion velocity vector as an x,y pair [(um/s,um/s)].'
                            = (0, 0),
            positionUnits   : "The unit of measurement for startPosition ('um', 'pixel', 'percent', or 'electrodeIndex')."
                            = 'um',
            startPosition   : 'Initial position as an x,y pair [(positionUnits,umpositionUnits)].'
                            = (0, 0),
            angularVelocity : 'Rotation velocity [rad/s].'
                            = 0,
            startAngle      : 'Initial orientation angle [rad].'
                            = 0,
            scaleVelocity   : 'Rate of scale factor change as an x,y pair [(1/s,1/s)].'
                            = (0, 0),
            startScale      : 'Initial scale factors as an x,y pair  [1, 1].'
                            = (1, 1)
            ) :
        sequence = spass.getSequence().getPythonObject()
        if isinstance(startPosition[0], str) or positionUnits == 'electrodeIndex':
            code = ord(startPosition[0])
            if code >= ord('J') :
                code -= 1
            code -= ord('A')
            startPosition = (sequence.electrodeOffset_um[0] + code * sequence.electrodeDistance_um[0],
                            sequence.electrodeOffset_um[1] + startPosition[1] * sequence.electrodeDistance_um[1])
        elif positionUnits == 'percent' :
            startPosition = ( sequence.field_width_um * startPosition[0], sequence.field_height_um * startPosition[1] )
        elif positionUnits == 'pixel' :
            startPosition = ( sequence.field_width_um * startPosition[0] / sequence.field_width_px, sequence.field_height_um * startPosition[1] / sequence.field_height_px )
        spass.setShaderVector( name = functionName + '_startPosition', x = startPosition[0], y=startPosition[1] )
        spass.setShaderVector( name = functionName + '_velocity', x = velocity[0], y=velocity[1] )
        if angularVelocity == 0  and startAngle == 0 and scaleVelocity == (0, 0) and startScale == (1, 1) :
            spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                mat3x2 @<X>@ (float time){ return mat3x2(vec2(1, 0), vec2(0, 1), -`startPosition - `velocity * time ); }
                ''' ).format( X = functionName ) )
        else:
            spass.setShaderVector( name = functionName + '_startScale', x = startScale[0], y=startScale[1] )
            spass.setShaderVector( name = functionName + '_scaleVelocity', x = scaleVelocity[0], y=scaleVelocity[1] )
            spass.setShaderVector( name = functionName + '_angleInitialAndVelocity', x = startAngle, y=angularVelocity )
       
            spass.setShaderFunction( name = functionName, src = self.glslEsc( '''
                mat3x2 @<X>@ (float time){
                    float angle = `angleInitialAndVelocity.x + `angleInitialAndVelocity.y * time;
                    vec2 cs = vec2(cos(angle), sin(angle));
                    vec2 scale = `startScale + `scaleVelocity * time;
                    vec2 pos = `startPosition + `velocity * time;
                    return mat3x2(
                        cs / scale,
                        cs.yx * vec2(-1, 1) / scale,
                        vec2( dot(pos, cs * vec2(-1, 1)) / scale.x, -dot(pos, cs.yx) / scale.y ) ); }
                ''' ).format( X = functionName ) )

    def applyForwardWithArgs(
            self,
            spass,
            functionName,
            *,
            velocity        : 'Motion velocity vector as an x,y pair [(um/s,um/s)].'
                            = (0, 0),
            startPosition   : 'Initial position as an x,y pair [(um,um)].'
                            = (0, 0),
            angularVelocity : 'Rotation velocity [rad/s].'
                            = 0,
            startAngle      : 'Initial orientation angle [rad].'
                            = 0,
            scaleVelocity   : 'Rate of scale factor change as an x,y pair [(1/s,1/s)].'
                            = (0, 0),
            startScale      : 'Initial scale factors as an x,y pair  [1, 1].'
                            = (1, 1)
            ) :
        sequence = spass.getSequence().getPythonObject()
        if isinstance(startPosition[0], str) :
            code = ord(startPosition[0])
            if code >= ord('J') :
                code -= 1
            code -= ord('A')
            startPosition = (sequence.electrodeOffset_um[0] + code * sequence.electrodeDistance_um[0],
                            sequence.electrodeOffset_um[1] + startPosition[1] * sequence.electrodeDistance_um[1])
        spass.setShaderVector( name = functionName + '_startPosition', x = startPosition[0], y=startPosition[1] )
        spass.setShaderVector( name = functionName + '_velocity', x = velocity[0], y=velocity[1] )
        if angularVelocity == 0  and startAngle == 0 and scaleVelocity == (0, 0) and startScale == (1, 1) :
            spass.setGeomShaderFunction( name = functionName, src = self.glslEsc( '''
                mat3x2 @<X>@ (float time){ return mat3x2(vec2(1, 0), vec2(0, 1), `startPosition + `velocity * time ); }
                ''' ).format( X = functionName ) )
        else:
            spass.setShaderVector( name = functionName + '_startScale', x = startScale[0], y=startScale[1] )
            spass.setShaderVector( name = functionName + '_scaleVelocity', x = scaleVelocity[0], y=scaleVelocity[1] )
            spass.setShaderVector( name = functionName + '_angleInitialAndVelocity', x = startAngle, y=angularVelocity )
       
            spass.setGeomShaderFunction( name = functionName, src = self.glslEsc( '''
                mat3x2 @<X>@ (float time){
                    float angle = `angleInitialAndVelocity.x + `angleInitialAndVelocity.y * time;
                    vec2 cs = vec2(cos(angle), sin(angle));
                    vec2 scale = `startScale + `scaleVelocity * time;
                    vec2 pos = `startPosition + `velocity * time;
                    return mat3x2(
                        cs * scale,
                        cs.yx * vec2(-1, 1) * scale,
                        pos ); }
                ''' ).format( X = functionName ) )


