import GearsModule as gears
from .. import * 

class XorShift128(Component) : 

    def applyWithArgs(self,
            stimulus,
            *,
            randomSeed: 'Number used to initialize the PRNG. The same seed always produces the same randoms.'=3773623027,
            randomGridSize: 'The dimensions of the 2D array of randoms generated, as an x,y pair.'=(41, 41)) :
        stimulus.rngCompute_workGroupSizeX = randomGridSize[0]
        stimulus.rngCompute_workGroupSizeY = randomGridSize[1]
        stimulus.rngCompute_seed = randomSeed
        stimulus.rngCompute_shaderSource = f"""
#version 450

layout (set = 0, binding = 0) uniform RandomGeneratorConfig {{
    uint seed;
    uint framesInFlight;
    
    uint startFrameIndex;
    uint nextElementIndex;
}};

layout (set = 0, binding = 1) buffer OutputBuffer {{
    uvec4 randomsBuffer[{stimulus.rngCompute_workGroupSizeY}][{stimulus.rngCompute_workGroupSizeX}][FRAMESINFLIGHT];
}};

void main()
{{
    uint gIDx = gl_GlobalInvocationID.x;
    uint gIDy = gl_GlobalInvocationID.y;

    uint framesToGenerate = min (startFrameIndex, framesInFlight);

    for (uint frame = startFrameIndex; frame < startFrameIndex + framesToGenerate; ++frame) {{
        uvec4 nextElement = uvec4 (0);
        if(frame == 1) {{
            nextElement.r = gIDx * 1341593453u ^ gIDy *  971157919u ^ seed * 2883500843u;
            nextElement.g = gIDx * 1790208463u ^ gIDy * 1508561443u ^ seed * 2321036227u;
            nextElement.b = gIDx * 2659567811u ^ gIDy * 2918034323u ^ seed * 2244239747u;
            nextElement.a = gIDx * 3756158669u ^ gIDy * 1967864287u ^ seed * 1275070309u;
        }} else if (frame == 2) {{
            nextElement.r = gIDx * 2771446331u ^ gIDy * 3030392353u ^ seed *  395945089u;
            nextElement.g = gIDx * 3459812197u ^ gIDy * 2853318569u ^ seed * 1233582347u;
            nextElement.b = gIDx * 2926663697u ^ gIDy * 2265556091u ^ seed * 3073622047u;
            nextElement.a = gIDx * 3459811891u ^ gIDy * 1756462801u ^ seed * 2805899363u;
        }} else if (frame == 3) {{
            nextElement.r = gIDx * 1470939049u ^ gIDy * 2244239737u ^ seed * 2056949767u;
            nextElement.g = gIDx * 1584004207u ^ gIDy * 1630196153u ^ seed * 2965533797u;
            nextElement.b = gIDx * 2248501561u ^ gIDy * 2728389799u ^ seed * 2099451241u;
            nextElement.a = gIDx *  715964407u ^ gIDy * 1735392947u ^ seed * 1496011453u;
        }} else if (frame == 4) {{
            nextElement.r = gIDx * 1579813297u ^ gIDy *  890180033u ^ seed * 1760681059u;
            nextElement.g = gIDx * 4132540697u ^ gIDy * 1362405383u ^ seed * 3052005647u;
            nextElement.b = gIDx * 3155894689u ^ gIDy * 1883169037u ^ seed * 2870559073u;
            nextElement.a = gIDx * 1883169037u ^ gIDy * 2278336279u ^ seed * 2278336133u;
        }} else {{
            uvec4 x = randomsBuffer[(nextElementIndex + frame - 1 + 1) % 5][gIDy][gIDx];
            uvec4 y = randomsBuffer[(nextElementIndex + frame - 1 + 2) % 5][gIDy][gIDx];
            uvec4 z = randomsBuffer[(nextElementIndex + frame - 1 + 3) % 5][gIDy][gIDx];
            uvec4 w = randomsBuffer[(nextElementIndex + frame - 1 + 4) % 5][gIDy][gIDx];
            // 128-bit xorshift algorithm
            uvec4 t = x ^ (x << 11u);
            nextElement = w ^ (w >> 19u) ^ t ^ (t >> 8u);
        }}
        randomsBuffer[(nextElementIndex + frame - 1) % framesInFlight][gIDy][gIDx] = nextElement;
    }}
}}"""
