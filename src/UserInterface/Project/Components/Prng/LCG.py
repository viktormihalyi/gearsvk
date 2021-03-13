import GearsModule as gears
from .. import * 

class LCG(Component):
    def applyWithArgs(self,
                      stimulus,
                      *,
                      randomSeed: 'Number used to initialize the PRNG. The same seed always produces the same randoms.'=1,
                      randomGridSize: 'The dimensions of the 2D array of randoms generated, as an x,y pair.'=(32, 32)):
        stimulus.randomGridWidth = randomGridSize[0]
        stimulus.randomGridHeight = randomGridSize[1]
        stimulus.randomSeed = randomSeed
        stimulus.randomGeneratorShaderSource = """
            layout (binding = 6) uniform ubo_lcg { uint seed; uint frameIndex; };
            
            layout (location = 1) in vec2 fTexCoord;

            layout (location = 0) out uvec4 nextElement;
            
            #extension GL_EXT_shader_explicit_arithmetic_types : enable

            uint64_t Forrest_C (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t c, const uint64_t m)
            {
                uint64_t C = seed % m;
                uint64_t f = c;
                uint64_t h = g;
                uint64_t i = (k + m) % m;

                while (i > 0) {
                    if (i % 2 == 1) {
                        C = (C * h + f) % m;
                    }
                    f = (f * (h + 1)) % m;
                    h = (h * h) % m;
                    i = i / 2;
                }

                return C;
            }

            void main ()
            {
                const vec2 randCoord = fTexCoord;

                uint32_t gridWidth  = @<gridSizeX>@;
                uint32_t gridHeight = @<gridSizeY>@;

                // each frame gets gridWidth*gridHeight random uvec4s

                const uint64_t gridFrameOffset = gridWidth * gridHeight * frameIndex * 4;
                const uint32_t k = gridFrameOffset + 4 * (frameIndex + uint (fTexCoord.x * gridWidth) * gridWidth + uint (fTexCoord.y * gridHeight));

                const uint64_t modulus = 2147483647;
                const uint64_t g       = 48271;
                const uint64_t c       = 0;
                const uint64_t seed    = 456;

                uint64_t rng_0 = Forrest_C (k + 0, seed, g, c, modulus);
                uint64_t rng_1 = Forrest_C (k + 1, seed, g, c, modulus);
                uint64_t rng_2 = Forrest_C (k + 2, seed, g, c, modulus);
                uint64_t rng_3 = Forrest_C (k + 3, seed, g, c, modulus);
                
                nextElement = uvec4 (rng_0, rng_1, rng_2, rng_3);
            }
		""".format(gridSizeX=randomGridSize[0], gridSizeY=randomGridSize)