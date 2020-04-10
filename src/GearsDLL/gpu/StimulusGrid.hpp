#ifndef _STIMULUSGRID_
#define _STIMULUSGRID_

#include "GearsDLL/OpenGLProxy.hpp"

class StimulusGrid {
private:
    GLuint handle;
    GLuint colorBuffer;
    GLuint width, height;

    GLenum buffer;

public:
    StimulusGrid (GLuint width, GLuint height, void* initData = 0);
    ~StimulusGrid ();

    void setRenderTarget ();
    void disableRenderTarget ();

    GLuint getColorBuffer ()
    {
        return colorBuffer;
    }

    int getWidth ()
    {
        return width;
    }

    int getHeight ()
    {
        return height;
    }

    GLuint getHandle ()
    {
        return handle;
    }
};

#endif
