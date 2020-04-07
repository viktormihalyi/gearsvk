import Gears as gears
from .. import * 

class StaticHash(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            randomSeed      : 'Number used to initialize the PRNG. The same seed always produces the same randoms.'
                            = 3773623027,
            randomGridSize  : 'The dimensions of the 2D array of randoms generated, as an x,y pair.'
                            = (41, 41)
            ) :
        stimulus.randomGridWidth =  randomGridSize[0]
        stimulus.randomGridHeight = randomGridSize[1]
        stimulus.randomSeed = randomSeed
        stimulus.freezeRandomsAfterFrame = 1
        stimulus.randomGeneratorShaderSource = """
            uniform usampler2D previousSequenceElements0;
            uniform usampler2D previousSequenceElements1;
            uniform usampler2D previousSequenceElements2;
            uniform usampler2D previousSequenceElements3;
            uniform uint seed;

            out uvec4 nextElement;

            void main() 
            {
                uvec2 p = uvec2(gl_FragCoord.xy);
                nextElement.r = p.x * 1341593453u ^ p.y *  971157919u ^ seed * 2883500843u;
                nextElement.g = p.x * 1790208463u ^ p.y * 1508561443u ^ seed * 2321036227u;
                nextElement.b = 0u;//p.x * 2659567811u ^ p.y * 2918034323u ^ seed * 2244239747u;
                nextElement.a = 0u;//p.x * 3756158669u ^ p.y * 1967864287u ^ seed * 1275070309u;
            }
		"""