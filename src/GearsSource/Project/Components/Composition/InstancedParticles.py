import Gears as gears
from .. import * 
from ..Pif.Base import *

class InstancedParticles(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            pifs             : 'List of PIFs the instances may use. (list of Pif.*)'
                                = [],
            backgroundColor     : 'Screen is cleared to this color before rendering the quads.'
                                = 0.0
            ) :
        backgroundColor = processColor(backgroundColor, self.tb)

        ffunc = '''
            in vec2 figmotid;
            vec3 @<X>@ (vec2 x, float time){
        '''
        stimulus = spass.getStimulus()
        stimulus.requiresClearing = True
        stimulus.setClearColor(red=backgroundColor[0], green=backgroundColor[1], blue=backgroundColor[2])
        i = 0
        for fig in pifs :
            fig.apply(spass, functionName + '_f' + str(i))
            ffunc += 'if(figmotid.x < {ipi}) return `f{i}(x, time);'.format(ipi = (i+1) / len(pifs), i=i)
            i += 1
        ffunc += '}'

        mfunc = '''
            vec2 motionTransform(vec2 x, float time, float motid, ivec2 iid) { 
        '''
        spass.setMotionTransformFunction( self.glslEsc( '''
            vec2 motionTransform(vec2 x, float time, float motid, ivec2 iid) { 
                    vec4 randpose = vec4(texelFetch(particles, iid, 0)) / float(0xffff) - vec4( float(0x8000), float(0x8000), float(0x8000), float(0x8000));
                    vec2 cs = vec2(cos(randpose.z), sin(randpose.z));
                    mat3x2 m = mat3x2(
                        cs  * randpose.w ,
                        cs.yx * vec2(-1, 1)  * randpose.w ,
                        randpose.xy );
                return m * vec3(x, 1);
            }
        ''' ).format( X=functionName ) )

        spass.setShaderFunction( name = functionName, src = self.glslEsc( ffunc ).format( X=functionName )  ) 

