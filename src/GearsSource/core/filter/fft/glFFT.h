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

#ifndef GLFFT_H
#define GLFFT_H

#include <functional>
#include "FFT.h"

class GLFFT : public FFT
{
public:
  GLFFT(unsigned int width, unsigned int height, 
      unsigned int input_tex = 0, bool inverse1 = false, bool inverse2 = false);
  ~GLFFT();

  // overrided methods
  virtual void do_fft() override;
  virtual unsigned int get_fullTex() const override { return fft[current_fft]; }
  virtual unsigned int take_fullTex_ownership() override { ownsTex = false; return fft[current_fft]; }
  virtual void redraw_input() override;

  // new methods
  void invert( int i = 1 );
  void draw_output(float r, float g, float b, int i = 1) const;
  bool newly_redrawn() const { return redrawn; }

private:
  bool inverse[2];
  unsigned short has_input_tex;  // GLboolean
  unsigned int stages[2];
  float* butterflyI[2];
  float* butterflyWR[2];
  float* butterflyWI[2];
  unsigned int* scramblers[2];
  unsigned int* real_weights[2];
  unsigned int* imag_weights[2];
  unsigned int disp_lists[2];
  unsigned int fft[2];   
  unsigned int current_fft;
  unsigned int fft_prog;
  unsigned int input_prog[2];
  unsigned int disp_prog[2];

  void test_glew() const;
  void create_butterfly_tables(int d);
  void init_texture(unsigned int tex, GLenum iformat, GLenum format, float* data, int d = 2);
  void init_textures(int d);
  void init_display_lists(int d);
  void init_framebuffer();
  void init_shaders();
  void set_projection() const;
  void draw_quad() const;
  void do_stage(int d, unsigned int s);
  void do_fft(int d);
};

#endif // GLFFT_H
