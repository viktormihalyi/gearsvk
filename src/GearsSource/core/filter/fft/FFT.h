#ifndef FFT_H
#define FFT_H

#include <functional>

enum class FFTChannelMode
{
	Monochrome,
	Multichrome
};

class FFT
{
public:
	FFT( unsigned int width, unsigned int height )
		:ownsTex(true), redrawn( false ) 
	{
		size[0] = width;
		size[1] = height;
	}
	virtual ~FFT() {}

	void set_input( std::function<void(int)> f ) { draw_input = f; has_draw_input = true; }

	virtual void do_fft() = 0;
	virtual unsigned int get_fullTex() const = 0;
	virtual unsigned int take_fullTex_ownership() = 0;
	virtual void redraw_input() = 0;
	virtual bool storeFrequencyInTexture() const { return true; }

protected:
	bool ownsTex;
	bool redrawn;
	unsigned int size[2];
	unsigned int fbo;
	std::function<void(int)> draw_input;
	bool has_draw_input = false;
};

#endif // FFT_H
