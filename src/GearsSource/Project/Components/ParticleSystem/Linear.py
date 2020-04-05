import Gears as gears
from .. import * 

class Linear(Component) : 

    def applyWithArgs(
            self,
            spass,
            *,
            particleGridSize    : 'The dimensions of the 2D array of particles, as an x,y pair.'
                                = (64, 64)
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
                    newParticle = oldParticle + ((random >> 20) - uvec4(0x1<< 11, 0x1<< 11, 0x1<< 11, 0x1 << 11)) * uvec4(100, 100, 0, 0);
                    if(newParticle.x < (0x1 << 11)-1000)    newParticle.x = (0x1 << 11)+1000;
                    //if(newParticle.x < -1000)    newParticle.x = 1000;
                    //if(newParticle.y > 1000)    newParticle.y = -1000;
                    //if(newParticle.y > -1000)    newParticle.y = 1000;
                }
            }
		''' ).format( dt=sequence.getFrameInterval_s() )