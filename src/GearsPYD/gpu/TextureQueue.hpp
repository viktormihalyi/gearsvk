#ifndef _StimulusQueueTextureArray_
#define _StimulusQueueTextureArray_

#include "OpenGLProxy.hpp"

class TextureQueue {
private:
    GLuint handle;
    GLuint colorBuffer;
    GLuint width, height, nSlices;

    GLenum buffer;

public:
    TextureQueue (GLuint width, GLuint height, GLuint nSlices, bool mono, bool integer);
    ~TextureQueue ();

    void setRenderTarget (GLuint slice);
    void disableRenderTarget ();

    void setRenderTargets ();
    void disableRenderTargets ();


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

    int getDepth ()
    {
        return nSlices;
    }

    GLuint getHandle ()
    {
        return handle;
    }

    void clear ();
};


#endif
