#version 150

uniform vec2 patternSizeOnRetina;
uniform sampler2D polymaskVertices;

out vec2 pos;

void main(void) {
   gl_Position = texelFetch(polymaskVertices, ivec2(gl_VertexID, 0));
   pos = gl_Position * patternSizeOnRetina * 0.5;
}

