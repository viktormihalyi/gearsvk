#ifndef OPENGLPROXY_HPP
#define OPENGLPROXY_HPP

#include <cstdint>
#include <exception>
#include <stdexcept>

using GLint     = int32_t;
using GLuint    = uint32_t;
using GLenum    = uint32_t;
using GLfloat   = float;
using GLboolean = bool;

constexpr GLenum GL_LINE_STRIP     = 0;
constexpr GLenum GL_TRIANGLE_STRIP = 0;

constexpr GLint GL_FALSE = 0;
constexpr GLint GL_TRUE  = 1;

constexpr GLenum GL_RGBA16F = 0;
constexpr GLenum GL_RGBA32F = 0;


#endif