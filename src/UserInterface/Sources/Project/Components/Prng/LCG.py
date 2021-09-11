import GearsModule as gears
from .. import * 

class LCG(Component):
    def applyWithArgs(self,
                      stimulus,
                      *,
                      randomSeed: 'Number used to initialize the PRNG. The same seed always produces the same randoms.'=1,
                      randomGridSize: 'The dimensions of the 2D array of randoms generated, as an x,y pair.'=(32, 32)):
        stimulus.rngCompute_workGroupSizeX = randomGridSize[0]
        stimulus.rngCompute_workGroupSizeY = randomGridSize[1]
        stimulus.rngCompute_seed = randomSeed
        stimulus.rngCompute_multiLayer = False
        stimulus.rngCompute_shaderSource = f"""
#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : enable

layout (binding = 6) uniform RandomGeneratorConfig {{
    uint seed;
    uint framesInFlight;
    
    uint startFrameIndex;
    uint nextElementIndex;
}};

layout (binding = 7) buffer OutputBuffer {{
    uvec4 randomsBuffer[{stimulus.rngCompute_workGroupSizeY}][{stimulus.rngCompute_workGroupSizeX}][FRAMESINFLIGHT];
}};

uint64_t Forrest_C (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t c, const uint64_t m)
{{
    uint64_t C = seed % m;
    uint64_t f = c;
    uint64_t h = g;
    uint64_t i = (k + m) % m;

    while (i > 0) {{
        if (i % 2 == 1) {{
            C = (C * h + f) % m;
        }}
        f = (f * (h + 1)) % m;
        h = (h * h) % m;
        i = i / 2;
    }}

    return C;
}}

void main ()
{{
    uint gridWidth  = gl_WorkGroupSize.x;
    uint gridHeight = gl_WorkGroupSize.y;
    
    const uint64_t frameOffset = gridWidth * gridHeight * 4 * startFrameIndex;
    const uint64_t pxOffset = uint (gl_GlobalInvocationID.y * gridWidth * gridHeight + gl_GlobalInvocationID.x * gridHeight) * 4;
    
    const float perc1 = float (Forrest_C (frameOffset + pxOffset + 0, seed, 48271, 0, 2147483647)) / float (2147483647);
    const float perc2 = float (Forrest_C (frameOffset + pxOffset + 1, seed, 48271, 0, 2147483647)) / float (2147483647);
    const float perc3 = float (Forrest_C (frameOffset + pxOffset + 2, seed, 48271, 0, 2147483647)) / float (2147483647);
    const float perc4 = float (Forrest_C (frameOffset + pxOffset + 3, seed, 48271, 0, 2147483647)) / float (2147483647);
    
    uvec4 nextElement = uvec4 (perc1 * uint (-1), perc2 * uint (-1), perc3 * uint (-1), perc4 * uint (-1));
    
    randomsBuffer[0][gl_GlobalInvocationID.y][gl_GlobalInvocationID.x] = nextElement;

    //const uint perc1 = uint(frameOffset) + uint(pxOffset) + 0;
    //const uint perc2 = uint(frameOffset) + uint(pxOffset) + 1;
    //const uint perc3 = uint(frameOffset) + uint(pxOffset) + 2;
    //const uint perc4 = uint(frameOffset) + uint(pxOffset) + 3;
    //nextElement = uvec4 (perc1, perc2, perc3, perc4);
}}"""
