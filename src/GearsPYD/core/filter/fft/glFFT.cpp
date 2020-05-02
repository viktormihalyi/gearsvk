// FFT class which computes two 2D fast Fourier transforms in parallel
// on the GPU.
//
// Assumes that GLEW is available and has been initialized.
//
// Implementation is based on T. Sumanaweera and D. Liu. Medical Image 
// Reconstruction with the FFT. In GPU Gems 2: Programming Techniques 
// for High-Performance Graphics and General-Purpose Computation, 
// Chapter 48, pp. 765-784, Addison-Wesley, 2005.
//
// Input image is provided as a texture or a callback function which
// draws the input.
//
// Code written by Jeppe Revall Frisvad, 2009
// Copyright (c) DTU Informatics 2009

#include "stdafx.h"
#include <iostream>
#include <cmath>    
#include <GL/glew.h>
#include "load_shaders.h"
#include "GLFFT.h"
#include <ctime>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

namespace
{
  GLchar* minimal_vert =
    "void main()\n"
    "{\n"
    "  gl_Position = ftransform();\n"
    "}";

  GLchar* fft_frag =
    "#extension GL_ARB_texture_rectangle : enable            \n"
    "uniform sampler2DRect fft;                              \n"
    "uniform sampler2DRect scrambler;                        \n"
    "uniform sampler2DRect real_weight;                      \n"
    "uniform sampler2DRect imag_weight;                      \n"
    "uniform int dimension;                                  \n"
    "uniform bvec2 inverse;                                  \n"

    "void main()                                             \n"
    "{                                                       \n"
    "  vec2 texcoord = gl_FragCoord.xy - vec2(0.5, 0.5);     \n"
    "  vec4 i = texture2DRect(scrambler, texcoord);          \n"
    "  vec4 wr = texture2DRect(real_weight, texcoord);       \n"
    "  vec4 wi = texture2DRect(imag_weight, texcoord);       \n"
    "  vec2 newcoord;                                        \n"
    "  newcoord.x = dimension == 0 ? i.r : texcoord.x;       \n"
    "  newcoord.y = dimension == 1 ? i.r : texcoord.y;       \n"
    "  vec4 Input1 = texture2DRect(fft, newcoord);           \n"
    "  newcoord.x = dimension == 0 ? i.a : texcoord.x;       \n"
    "  newcoord.y = dimension == 1 ? i.a : texcoord.y;       \n"
    "  vec4 Input2 = texture2DRect(fft, newcoord);           \n"   
    "  vec4 Res;                                             \n"
    "  float imag = inverse.x ? -wi.a : wi.a;                \n"
    "  Res.x = wr.a*Input2.x - imag*Input2.y;                \n"
    "  Res.y = imag*Input2.x + wr.a*Input2.y;                \n"
    "  imag = inverse.y ? -wi.a : wi.a;                      \n"
    "  Res.z = wr.a*Input2.z - imag*Input2.w;                \n"
    "  Res.w = imag*Input2.z + wr.a*Input2.w;                \n"
    "  Res += Input1;                                        \n"
    "  Res.xy = inverse.x ? Res.xy/2.0 : Res.xy;             \n"
    "  Res.zw = inverse.y ? Res.zw/2.0 : Res.zw;             \n"
    "  gl_FragColor = Res;                                   \n"
    "}";

  GLchar* position_vert =
    "varying vec2 position;\n"
    "uniform vec2 size;\n"
    "void main()\n"
    "{\n"
    "  position = gl_Vertex.xy + size*0.5;\n"
    "  gl_Position = ftransform();\n"
    "}";

  GLchar* display1_frag =
    "#extension GL_ARB_texture_rectangle : enable            \n"
    "varying vec2 position;                                  \n"
    "uniform sampler2DRect fft;                              \n"
    "uniform vec2 size;                                      \n"
    "uniform vec3 color;                                     \n"
    "void main()                                             \n"
    "{                                                       \n"
    "  vec2 texcoord = mod(position, size);                  \n"
    "  vec4 ffts = texture2DRect(fft, texcoord);             \n"
    "  gl_FragColor.r = length(ffts.rg);                     \n" 
    "  gl_FragColor.g = gl_FragColor.r;                      \n"
    "  gl_FragColor.b = gl_FragColor.r;                      \n"
    "  gl_FragColor.rgb *= color;                            \n"
    "}";

  GLchar* display2_frag =
    "#extension GL_ARB_texture_rectangle : enable            \n"
    "varying vec2 position;                                  \n"
    "uniform sampler2DRect fft;                              \n"
    "uniform vec2 size;                                      \n"
    "uniform vec3 color;                                     \n"
    "void main()                                             \n"
    "{                                                       \n"
    "  vec2 texcoord = mod(position, size);                  \n"
    "  vec4 ffts = texture2DRect(fft, texcoord);             \n"
    "  gl_FragColor.r = length(ffts.ba);                     \n" 
    "  gl_FragColor.g = gl_FragColor.r;                      \n"
    "  gl_FragColor.b = gl_FragColor.r;                      \n"
    "  gl_FragColor.rgb *= color;                            \n"
    "}";

  GLchar* input_vert =
    "varying vec2 position;\n"
    "void main()\n"
    "{\n"
    "  position = gl_Vertex.xy;\n"
    "  gl_Position = ftransform();\n"
    "}";

  GLcharARB* input1_frag =
    "#extension GL_ARB_texture_rectangle : enable            \n"
    "varying vec2 position;                                  \n"
    "uniform sampler2DRect fft;                              \n"
    "void main()                                             \n"
    "{                                                       \n"
    "  vec4 ffts = texture2DRect(fft, position.xy);          \n"
    "  gl_FragColor.rgb = vec3(length(ffts.rg));             \n" 
    "}";

  GLchar* input2_frag =
    "#extension GL_ARB_texture_rectangle : enable            \n"
    "varying vec2 position;                                  \n"
    "uniform sampler2DRect fft;                              \n"
    "void main()                                             \n"
    "{                                                       \n"
    "  vec4 ffts = texture2DRect(fft, position.xy);          \n"
    "  gl_FragColor.rgb = vec3(length(ffts.ba));             \n" 
    "}";

  int bit_reverse(int i, int N)
  {
    int j = 0;
    while(N = N>>1)
    {
      j = (j<<1) | (i&1);
      i = i>>1;
    }
    return j;
  }
}

GLFFT::GLFFT(unsigned int width, unsigned int height, 
         unsigned int input_tex, bool inv1, bool inv2) 
  : FFT(width, height), current_fft(0)
{
  test_glew();

  // Two parallel FFTs
  has_input_tex = glIsTexture(input_tex);
  if(has_input_tex) fft[0] = input_tex;
  inverse[0] = inv1;
  inverse[1] = inv2;

  // Two dimensions
  // size[0] = width;
  // size[1] = height;
  for(int i = 0; i < 2; ++i)
  {
    unsigned int s = size[i];
    stages[i] = 0;
    while(s = s>>1)
      ++stages[i];
    butterflyI[i] = new float[2*size[i]*stages[i]];
    butterflyWR[i] = new float[size[i]*stages[i]];
    butterflyWI[i] = new float[size[i]*stages[i]];
    scramblers[i] = new unsigned int[stages[i]];
    real_weights[i] = new unsigned int[stages[i]];
    imag_weights[i] = new unsigned int[stages[i]];
    create_butterfly_tables(i);
    init_textures(i);
    init_display_lists(i);
  }
  init_framebuffer();
  init_shaders();
}

GLFFT::~GLFFT()
{
	for(int i = 0; i < 2; ++i)
	{
		glDeleteTextures(stages[i], scramblers[i]);
		glDeleteTextures(stages[i], real_weights[i]);
		glDeleteTextures(stages[i], imag_weights[i]);
	}
	if(ownsTex)
		glDeleteTextures(2, fft);
	else
		glDeleteTextures(1, &fft[1-current_fft]);
	glDeleteLists(disp_lists[0], 1);
	glDeleteLists(disp_lists[1], 1);

	for(int i = 0; i < 2; ++i)
	{
		delete[] butterflyI[i];
		delete[] butterflyWR[i];
		delete[] butterflyWI[i];
		delete[] scramblers[i];
		delete[] real_weights[i];
		delete[] imag_weights[i];
	}
}

void GLFFT::test_glew() const
{
  if(!GLEW_VERSION_2_1)
  {
    cerr << "Warning: OpenGL version 2.1 is not supported" << endl;
  }

  if(!GLEW_ARB_texture_rectangle)
  {
    cerr << "Error: Rectangular textures are not supported" << endl;
    exit(0);
  }

  if(!GLEW_ARB_fragment_program)
  {
    cerr << "Error: Fragment programs are not supported" << endl;
    exit(0);
  }

  if(!GLEW_ARB_texture_float)
  {
    cerr << "Error: Floating point textures are not supported" << endl;
    exit(0);
  }

  if(!GLEW_EXT_framebuffer_object)
  {
    cerr << "Error: Framebuffer objects are not supported" << endl;
    exit(0);
  }
}

void GLFFT::create_butterfly_tables(int d)
{
  int n = 0;
  for(unsigned int i = 0; i < stages[d]; ++i)
  {
    int blocks = 1<<(stages[d] - 1 - i);
    int block_inputs = 1<<i;
    for(int j = 0; j < blocks; ++j)
      for(int k = 0; k < block_inputs; ++k)
      {
        int i1 = j*block_inputs*2 + k;
        int i2 = i1 + block_inputs;
        float j1, j2;
        if(i == 0)
        {
          j1 = static_cast<float>(bit_reverse(i1, size[d]));
          j2 = static_cast<float>(bit_reverse(i2, size[d]));
        }
        else
        {
          j1 = static_cast<float>(i1);
          j2 = static_cast<float>(i2);
        }
        i1 += n;
        i2 += n;
        butterflyI[d][2*i1] = j1;
        butterflyI[d][2*i1 + 1] = j2;
        butterflyI[d][2*i2] = j1;
        butterflyI[d][2*i2 + 1] = j2;

        // Compute weights
        double angle = 2.0*M_PI*k*blocks/static_cast<float>(size[d]);
        float wr = static_cast<float>( cos(angle));
        float wi = static_cast<float>(-sin(angle));

        butterflyWR[d][i1] = wr;
        butterflyWI[d][i1] = wi;
        butterflyWR[d][i2] = -wr;
        butterflyWI[d][i2] = -wi;
      }
    n += size[d];
  }
}

void GLFFT::init_texture(unsigned int tex, GLenum iformat, GLenum format, float* data, int d)
{
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, iformat, d == 1 ? 1 : size[0], d == 0 ? 1 : size[1], 0, format, GL_FLOAT, data);        
}

void GLFFT::init_textures(int d)
{
  glGenTextures(stages[d], scramblers[d]);
  glGenTextures(stages[d], real_weights[d]);
  glGenTextures(stages[d], imag_weights[d]);
  for(unsigned int i = 0; i < stages[d]; ++i)
  {
    init_texture(scramblers[d][i], GL_LUMINANCE_ALPHA16F_ARB, GL_LUMINANCE_ALPHA, &butterflyI[d][i*2*size[d]], d);
    init_texture(real_weights[d][i], GL_ALPHA32F_ARB, GL_ALPHA, &butterflyWR[d][i*size[d]], d);
    init_texture(imag_weights[d][i], GL_ALPHA32F_ARB, GL_ALPHA, &butterflyWI[d][i*size[d]], d);
  }
}

void GLFFT::init_framebuffer()
{
  if(!has_input_tex)
  {
    glGenTextures(2, fft);
    init_texture(fft[0], GL_RGBA32F_ARB, GL_RGBA, 0);    
    init_texture(fft[1], GL_RGBA32F_ARB, GL_RGBA, 0);    
  }
  else
  {
    glGenTextures(1, &fft[1]);
    init_texture(fft[1], GL_RGBA32F_ARB, GL_RGBA, 0);    
  }

  glGenFramebuffersEXT(1, &fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, fft[0], 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, fft[1], 0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void GLFFT::init_shaders()
{
  load_shaders(minimal_vert, fft_frag, fft_prog);
  load_shaders(position_vert, display1_frag, disp_prog[0]);
  load_shaders(position_vert, display2_frag, disp_prog[1]);
  load_shaders(input_vert, input1_frag, input_prog[0]);
  load_shaders(input_vert, input2_frag, input_prog[1]);
  glUseProgram(fft_prog);  
  glUniform1i(glGetUniformLocation(fft_prog, "fft"), 0);
  glUniform1i(glGetUniformLocation(fft_prog, "scrambler"), 1);       
  glUniform1i(glGetUniformLocation(fft_prog, "real_weight"), 2);
  glUniform1i(glGetUniformLocation(fft_prog, "imag_weight"), 3);
  glUniform1i(glGetUniformLocation(fft_prog, "dimension"), 0);
  glUniform2i(glGetUniformLocation(fft_prog, "inverse"), inverse[0], inverse[1]);
  for(int i = 0; i < 2; ++i)
  {
    glUseProgram(disp_prog[i]);
    glUniform1i(glGetUniformLocation(disp_prog[i], "fft"), 0);
    glUniform2f(glGetUniformLocation(disp_prog[i], "size"), static_cast<float>(size[0]), static_cast<float>(size[1]));
    glUseProgram(input_prog[i]);
    glUniform1i(glGetUniformLocation(input_prog[i], "fft"), 0);
  }
  glUseProgram(0);
}

void GLFFT::init_display_lists(int d)
{
  disp_lists[d] = glGenLists(stages[d]);
  for(unsigned int i = 0; i < stages[d]; ++i)
  {
    glNewList(disp_lists[d] + i, GL_COMPILE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, scramblers[d][i]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, real_weights[d][i]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, imag_weights[d][i]);

    draw_quad();  

    glEndList();
  }
}

void GLFFT::set_projection() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, static_cast<double>(size[0]), 0.0, static_cast<double>(size[1]), -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}

void GLFFT::draw_quad() const
{
  glBegin(GL_POLYGON);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(size[0]), 0.0f);
    glVertex2f(static_cast<float>(size[0]), static_cast<float>(size[1]));
    glVertex2f(0.0f, static_cast<float>(size[1]));
  glEnd();
}

void GLFFT::do_stage(int d, unsigned int s)
{
  unsigned int render_fft = !current_fft;
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + render_fft);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, fft[current_fft]);

  glCallList(disp_lists[d] + s);

  current_fft = render_fft;
}

void GLFFT::do_fft(int d)
{
  for(unsigned int i = 0; i < stages[d]; ++i)
    do_stage(d, i);
}

void GLFFT::do_fft( )
{
  int vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  glViewport(0, 0, size[0], size[1]);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

  if(!redrawn)
  {
    if(!has_input_tex && has_draw_input)
    {
      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + current_fft);
      draw_input(1);
    }
  }
  else
    redrawn = false;

  glUseProgram(fft_prog);
  for(int i = 0; i < 4; ++i)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
  }

  set_projection();

  for(int i = 0; i < 2; ++i)
  {
    glUniform1i(glGetUniformLocation(fft_prog, "dimension"), i);
    do_fft(i);
  }

  for(int i = 3; i >= 0; --i)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
  }
  glUseProgram(0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glViewport(vp[0], vp[1], vp[2], vp[3]);
}

void GLFFT::redraw_input()
{
  if(has_input_tex)
    current_fft = 0;
  else if(has_draw_input)
  {
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glViewport(0, 0, size[0]+1, size[1]+1);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + current_fft);
    draw_input(0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glViewport(vp[0], vp[1], vp[2], vp[3]);
    redrawn = true;
  }
}

void GLFFT::draw_output(float r, float g, float b, int i) const
{  
  if(redrawn)
    glUseProgram(i == 2 ? input_prog[1] : input_prog[0]);
  else    
  {
    unsigned int program = i == 2 ? disp_prog[1] : disp_prog[0];
    glUseProgram(program);
    glUniform3f(glGetUniformLocation(program, "color"), r, g, b);
  }
  set_projection();

  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, fft[current_fft]);

  draw_quad();

  glDisable(GL_TEXTURE_RECTANGLE_ARB);
  glUseProgram(0);
}

void GLFFT::invert(int i)
{
  inverse[i - 1] = !inverse[i - 1];
  glUseProgram(fft_prog);
  glUniform2i(glGetUniformLocation(fft_prog, "inverse"), inverse[0], inverse[1]);
  glUseProgram(0);
}
