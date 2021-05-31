#version 450
#extension GL_EXT_geometry_shader : enable



void main ()
{
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position.x = 1.f;
	EmitVertex();
	EndPrimitive();
}