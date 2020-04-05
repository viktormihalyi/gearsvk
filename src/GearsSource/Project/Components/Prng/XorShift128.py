import Gears as gears
from .. import * 

class XorShift128(Component) : 

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
                if(frame == 1) {
                    nextElement.r = p.x * 1341593453u ^ p.y *  971157919u ^ seed * 2883500843u;
                    nextElement.g = p.x * 1790208463u ^ p.y * 1508561443u ^ seed * 2321036227u;
                    nextElement.b = p.x * 2659567811u ^ p.y * 2918034323u ^ seed * 2244239747u;
                    nextElement.a = p.x * 3756158669u ^ p.y * 1967864287u ^ seed * 1275070309u;
                } else if (frame == 2) {
                    nextElement.r = p.x * 2771446331u ^ p.y * 3030392353u ^ seed *  395945089u;
                    nextElement.g = p.x * 3459812197u ^ p.y * 2853318569u ^ seed * 1233582347u;
                    nextElement.b = p.x * 2926663697u ^ p.y * 2265556091u ^ seed * 3073622047u;
                    nextElement.a = p.x * 3459811891u ^ p.y * 1756462801u ^ seed * 2805899363u;
                } else if (frame == 3) {
                    nextElement.r = p.x * 1470939049u ^ p.y * 2244239737u ^ seed * 2056949767u;
                    nextElement.g = p.x * 1584004207u ^ p.y * 1630196153u ^ seed * 2965533797u;
                    nextElement.b = p.x * 2248501561u ^ p.y * 2728389799u ^ seed * 2099451241u;
                    nextElement.a = p.x *  715964407u ^ p.y * 1735392947u ^ seed * 1496011453u;
                } else if (frame == 4) {
                    nextElement.r = p.x * 1579813297u ^ p.y *  890180033u ^ seed * 1760681059u;
                    nextElement.g = p.x * 4132540697u ^ p.y * 1362405383u ^ seed * 3052005647u;
                    nextElement.b = p.x * 3155894689u ^ p.y * 1883169037u ^ seed * 2870559073u;
                    nextElement.a = p.x * 1883169037u ^ p.y * 2278336279u ^ seed * 2278336133u;
                } else {
	                uvec4 x = texelFetch(previousSequenceElements3, ivec2(gl_FragCoord.xy), 0);
	                uvec4 y = texelFetch(previousSequenceElements2, ivec2(gl_FragCoord.xy), 0);
	                uvec4 z = texelFetch(previousSequenceElements1, ivec2(gl_FragCoord.xy), 0);
	                uvec4 w = texelFetch(previousSequenceElements0, ivec2(gl_FragCoord.xy), 0);
                    // 128-bit xorshift algorithm
                    uvec4 t = x ^ (x << 11u);
                    nextElement = w ^ (w >> 19u) ^ t ^ (t >> 8u);
                }
                if(frame < 5)
                {
                  uvec4 n = (nextElement << 13) ^ nextElement;
                  nextElement = n * (n*n*31069u+154933u) + 2935297931u;
                }
            }
		"""