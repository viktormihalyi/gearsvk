import Gears as gears
from .. import * 
from ..Pif.Base import *

class Instanced(Base) : 

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            pifs                : 'List of PIFs the instances may use. (list of Pif.*)'
                                = [],
            motions             : 'List of motions the instances may use. (list of Motion.*)'
                                = [],
            backgroundColor     : 'Screen is cleared to this color before rendering the quads.'
                                = 0.5
            ) :
        backgroundColor = processColor(backgroundColor, self.tb)

        ffunc = '''
            in vec2 figmotid;
            vec3 @<X>@ (vec2 x, float time){
        '''
        stimulus = spass.getStimulus()
        stimulus.requiresClearing = True
        stimulus.setClearColor(red=backgroundColor[0], green=backgroundColor[1], blue=backgroundColor[2])
        spass.transparent = True

        i = 0
        for fig in pifs :
            fig.apply(spass, functionName + '_f' + str(i))
            ffunc += 'if(figmotid.x < {ipi}) return `f{i}(x, time);'.format(ipi = (i+1) / len(pifs), i=i)
            i += 1
        ffunc += '}'

        mfunc = '''
            vec2 motionTransform(vec2 x, float time, float motid, ivec2 iid) { 
        '''
        j = 0
        for mot in motions :
            mot.applyForward(spass, functionName + '_m' + str(j))
            mfunc += 'if(motid < {ipi}) return `m{i}(time) * vec3(x, 1);'.format(ipi = (j+1) / len(motions), i=j)
            j += 1
        mfunc += '}'

        spass.setMotionTransformFunction( self.glslEsc( mfunc ).format( X=functionName ) )

        spass.setShaderFunction( name = functionName, src = self.glslEsc( ffunc ).format( X=functionName )  ) 

