#ifndef _RANDOMSEQUENCEBUFFER_
#define _RANDOMSEQUENCEBUFFER_

#include "GearsSource/OpenGLProxy.hpp"

class RandomSequenceBuffer {
private:
    GLuint handle;
    GLuint colorBuffer;
    GLuint width, height;

    GLenum buffer;

public:
    RandomSequenceBuffer (GLuint width, GLuint height, void* initData = 0);
    ~RandomSequenceBuffer ();

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
