#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

layout (std140, binding = 0) uniform _ {
    mat4 MVP;
} MVP;

layout (location = 0) out vec2 texCoord_o;

void main(void) {
   gl_Position = MVP.MVP * vec4 (position, 0.5, 1);
   texCoord_o = texCoord;
}

