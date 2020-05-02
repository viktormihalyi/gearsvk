// Simplistic program for loading GLSL shaders.
//
// Assumes that GLEW is available and has been initialized.
//
// Code written by Jeppe Revall Frisvad, 2009
// Copyright (c) DTU Informatics 2009

#include <iostream>
#include <GL/glew.h>
#include <boost/python.hpp>
#include "load_shaders.h"

using namespace std;

void print_shader_log(GLuint shader)
{
	GLint infoLog_length = 0;
	GLint chars_written = 0;
	GLchar *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLog_length);
	if(infoLog_length > 1)
	{
		infoLog = new GLchar[infoLog_length];
		if(!infoLog)
		{
			std::stringstream ss;
			ss << "OpenGL Error: Could not allocate infoLog buffer." << std::endl;
			std::cerr << ss.str();
			PyErr_SetString(PyExc_RuntimeError, ss.str().c_str());
			boost::python::throw_error_already_set();
		}
		glGetShaderInfoLog(shader, infoLog_length, &chars_written, infoLog);
		std::stringstream ss;
		ss << "InfoLog:" << endl << infoLog << std::endl;
		PyErr_Warn(PyExc_Warning, ss.str().c_str());

		delete[] infoLog;
	}
}

bool load_shaders(const char* vert_shader, const char* frag_shader, unsigned int& prog)
{
	GLuint vs, fs;                       // handles for shaders
	GLint vert_compiled, frag_compiled;  // status values
	GLint linked;

	// Create shaders
	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);

	// Load source code strings into shaders
	glShaderSource(vs, 1, &vert_shader, 0);
	glShaderSource(fs, 1, &frag_shader, 0);

	// Compile the vertex shader and print out the compiler log
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &vert_compiled);
	print_shader_log(vs);

	// Compile the fragment shader and print out the compiler log
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &frag_compiled);
	print_shader_log(fs);

	if(!vert_compiled || !frag_compiled)
		return false;

	// Create a program and attach the two compiled shaders
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	// Link the program
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if(!linked)
		return false;
	return true;
}
