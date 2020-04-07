// Simplistic program for loading GLSL shaders.
//
// Assumes that GLEW is available and has been initialized.
//
// Code written by Jeppe Revall Frisvad, 2009
// Copyright (c) DTU Informatics 2009

#ifndef LOAD_SHADERS_H
#define LOAD_SHADERS_H

bool load_shaders(const char* vert_shader, const char* frag_shader, unsigned int& prog);

#endif // LOAD_SHADERS_H
