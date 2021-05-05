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
            layout (binding = 6) uniform ubo_lcg { uint seed; uint frameIndex; uint gridSizeX; uint gridSizeY; };
            
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
                uint32_t gridWidth  = gridSizeX;
                uint32_t gridHeight = gridSizeY;
                
                const uint64_t frameOffset = gridWidth * gridHeight * 4 * frameIndex;
                const uint64_t pxOffset = uint (fTexCoord.y * gridWidth * gridHeight + fTexCoord.x * gridHeight) * 4;
                
                const float perc1 = float (Forrest_C (frameOffset + pxOffset + 0, seed, 48271, 0, 2147483647)) / float (2147483647);
                const float perc2 = float (Forrest_C (frameOffset + pxOffset + 1, seed, 48271, 0, 2147483647)) / float (2147483647);
                const float perc3 = float (Forrest_C (frameOffset + pxOffset + 2, seed, 48271, 0, 2147483647)) / float (2147483647);
                const float perc4 = float (Forrest_C (frameOffset + pxOffset + 3, seed, 48271, 0, 2147483647)) / float (2147483647);
                
                nextElement = uvec4 (perc1 * uint (-1), perc2 * uint (-1), perc3 * uint (-1), perc4 * uint (-1));
                
                //const uint perc1 = uint(frameOffset) + uint(pxOffset) + 0;
                //const uint perc2 = uint(frameOffset) + uint(pxOffset) + 1;
                //const uint perc3 = uint(frameOffset) + uint(pxOffset) + 2;
                //const uint perc4 = uint(frameOffset) + uint(pxOffset) + 3;
                //nextElement = uvec4 (perc1, perc2, perc3, perc4);
            }
		"""