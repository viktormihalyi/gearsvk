#version 150
#extension GL_EXT_geometry_shader : enable

void main(void) {
	gl_Position = gl_PositionIn[0];
	EmitVertex();
	gl_Position.x=1;
	EmitVertex();
	EndPrimitive();
}