#ifndef _TEXTURE_
#define _TEXTURE_

#include <string>

#include "GearsSource/OpenGLProxy.hpp"

class Texture1D {
private:
    GLuint handle;
    GLuint width;

public:
    Texture1D ();
    ~Texture1D ();

    int getWidth () { return width; }

    void initialize (GLuint width);
    void setData (const float* data);

    GLuint getTextureHandle ();
};

class Texture2D {
private:
    GLuint handle;
    GLuint width, height;
    GLuint bpp;

public:
    Texture2D ();
    ~Texture2D ();

    int getWidth () { return width; }
    int getHeight () { return height; }

    void initializeGrey (GLuint width, GLuint height, GLuint bpp);
    void setDataGrey (const unsigned char* data);

    void initialize8bit (GLuint width, GLuint height, GLuint bpp);
    void setData8bit (const unsigned char* data);


    void initialize (GLuint width, GLuint height, GLuint bpp);
    void setData (const float* data);
    void setData (const float* data, GLuint level);

    void   initializeRG (GLuint width, GLuint height, GLuint bpp);
    void   setDataRG (const float* data);
    void   loadFromFile (std::string fileName);
    GLuint getTextureHandle ();
};

#endif
