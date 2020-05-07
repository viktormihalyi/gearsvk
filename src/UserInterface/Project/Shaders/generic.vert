#version 450

layout (binding = 0) uniform PatternSizeOnRetina {
	vec2 value;
} patternSizeOnRetina;

layout (location = 0) out vec2 pos;
layout (location = 1) out vec2 fTexCoord;

void main(void) {
   gl_Position	= vec4(float(gl_VertexIndex / 2)*2.0-1.0, 1.0-float(gl_VertexIndex % 2)*2.0, 0.5, 1.0);
   vec2 texCoord = vec2(float(gl_VertexIndex / 2), float(gl_VertexIndex % 2));
   fTexCoord = texCoord;
   fTexCoord.y = -fTexCoord.y;
   fTexCoord.y += 1;
   pos = (texCoord - vec2(0.5, 0.5)) * patternSizeOnRetina.value;
}