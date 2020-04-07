import Gears as gears
from .. import * 

class Brownian(Component) : 

    def applyWithArgs(
            self,
            spass,
            *,
            particleGridSize    : 'The dimensions of the 2D array of particles, as an x,y pair.'
                                = (64, 64),
            diffusion           : 'The diffusion coefficient for displacement [um^2/s].'
                                = 100,
            angularDiffusion    : 'The diffusion coefficient for rotation [rad^2/s].'
                                = 1,
            scaleDiffusion      : 'The diffusion coefficient for scale [1/s].'
                                = 0.03
            ) :
        sequence = spass.getSequence().getPythonObject()
        spass.particleGridWidth =  particleGridSize[0]
        spass.particleGridHeight = particleGridSize[1]
        spass.particleShaderSource = self.glslEsc( '''
            uniform usampler2D randoms;
            uniform usampler2D previousParticles;

            out uvec4 newParticle;

            void main() 
            {
                uvec4 oldParticle = texelFetch(previousParticles, ivec2(gl_FragCoord.xy), 0);
                uvec4 random = texelFetch(randoms, ivec2(gl_FragCoord.xy), 0);
                if(frame == 1)
                    newParticle = uvec4(0x80000000, 0x80000000, 0x80000000, 0x80000000);
                else
                {
                    float dx = - 2 * @<diffusion>@ * @<dt>@;
                    float dr = - 2 * @<angularDiffusion>@ * @<dt>@;
                    float ds = - 2 * @<scaleDiffusion>@ * @<dt>@;

                    vec4 unitRand = random / float(0xffff) / float(0xffff);
                    unitRand.yw *= 6.283185307179586476925286766559;
                    unitRand.xz *= 0.999;
                    vec4 gaussian = vec4(
                        sqrt( dx * log(unitRand.x) ) * cos( unitRand.y ),
                        sqrt( dx * log(unitRand.x) ) * sin( unitRand.y ),
                        sqrt( dr * log(unitRand.z) ) * cos( unitRand.w ),
                        sqrt( ds * log(unitRand.z) ) * sin( unitRand.w ) );

                    //newParticle = oldParticle + ((random >> 20) - uvec4(0x1<< 11, 0x1<< 11, 0x1<< 11, 0x1 << 11)) * uvec4(dx / 0xffff, 100, 10, 1);
                    newParticle = uvec4(oldParticle + gaussian * 0xffff);
                }
            }
		''' ).format( diffusion = diffusion, angularDiffusion=angularDiffusion, scaleDiffusion=scaleDiffusion, dt=sequence.getFrameInterval_s() )