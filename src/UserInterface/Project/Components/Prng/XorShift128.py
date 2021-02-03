import GearsModule as gears
from .. import * 

class XorShift128(Component) : 

    def applyWithArgs(self,
            stimulus,
            *,
            randomSeed: 'Number used to initialize the PRNG. The same seed always produces the same randoms.'=3773623027,
            randomGridSize: 'The dimensions of the 2D array of randoms generated, as an x,y pair.'=(41, 41)) :
        stimulus.randomGridWidth = randomGridSize[0]
        stimulus.randomGridHeight = randomGridSize[1]
        stimulus.randomSeed = randomSeed
        stimulus.randomGeneratorShaderSource = """
            layout (binding = 6) uniform ubo_seed { uint seed; };
            
            layout (location = 1) in vec2 fTexCoord;

            layout (location = 0) out uvec4 nextElement;

            float rand (vec2 co)
            {
                return fract (sin (dot (co.xy, vec2 (12.9898, 78.233))) * 43758.5453);
            }

            void main ()
            {
                vec2 randCoord = fTexCoord;
                randCoord += vec2 (float (seed), 0.f);
                nextElement = uvec4 (rand (randCoord) > 0.5f ? 1 : 0);
            }
		"""