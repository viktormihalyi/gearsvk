#include "stdafx.h"
#include "openCLCore.h"

namespace ImageHelper
{

#ifdef _DEBUG
	void printPixel(std::ostream& st, const float* img, unsigned& idx, unsigned channels, bool complex, unsigned channel = 0)
	{
		/*if (abs(img[idx]) < 0.0000001f)
		{
			idx += channels;
			return;
		}*/
		st << "(";
		for ( unsigned k = 0; k < channels; k++ )
		{
			if (channel > 0 && channel != k + 1)
			{
				idx++;
				if (complex)
					idx++;
				continue;
			}
			st << img[idx++];
			if ( complex )
				st << (img[idx] >= 0.f ? "+" : "") << img[idx++] << "i";
			if ( channel <= 0 && k < channels - 1 )
				st << ", ";
		}
		st << ") ";
	}
#endif

	void _printImg( const float* img, unsigned w, unsigned h, const char* name, unsigned channels, bool complex, unsigned pad, unsigned channel, unsigned offsetW, unsigned offsetH, std::ostream& st )
	{
#ifdef _DEBUG
		if (w < offsetW || h < offsetH)
			return;
		unsigned idx = 0;
		st << name << ": " << std::endl;
		/*if ( w - offsetW > 10 )
			st << "Image too large, show only the first 10x10 submatrix!" << std::endl;*/
		for ( unsigned i = offsetH; i < h; i++ )
		{
			for ( unsigned j = offsetW; j < w; j++ )
			{
				printPixel(st, img, idx, channels, complex, channel);
			}
			if ( pad )
			{
				st << "| ";
				for ( unsigned k = 0; k < pad; k++ )
				{
					printPixel(st, img, idx, channels, complex, channel);
				}
			}
			st << std::endl;
		}
		st << std::endl;
		st << std::endl;
#endif
	}

	void printImg(const float* img, unsigned w, unsigned h, const char* name, unsigned channels, bool complex, unsigned pad, unsigned offsetW, unsigned offsetH)
	{
		_printImg( img, w, h, name, channels, complex, pad, 0, offsetW, offsetH, std::cout );
	}

	void printImgStream(std::ostream& st, const float* img, unsigned w, unsigned h, const char* name, unsigned channels, bool complex, unsigned pad, unsigned offsetW, unsigned offsetH)
	{
		_printImg(img, w, h, name, channels, complex, pad, 0, offsetW, offsetH, st);
	}

	void printImgChannel(  const float* img, unsigned w, unsigned h, unsigned channels, unsigned channel, const char* name, bool complex, unsigned pad, unsigned offsetW, unsigned offsetH)
	{
		_printImg( img, w, h, name, channels, complex, pad, channel, offsetW, offsetH, std::cout );
	}
}