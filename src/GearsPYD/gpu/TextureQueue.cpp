
#include "TextureQueue.hpp"

#include "Assert.hpp"

#include <cmath>
#include <iostream>
#include <string>
//#include<boost / python.hpp>

TextureQueue::TextureQueue (GLuint width, GLuint height, GLuint nSlices, bool mono, bool integer)
{
    this->width   = width;
    this->height  = height;
    this->nSlices = nSlices;

    GVK_BREAK ("unused func");
#if 0
    glGenFramebuffers (1, &handle);
    glGenTextures (1, &colorBuffer);
    glBindFramebuffer (GL_FRAMEBUFFER, handle);

    glBindTexture (GL_TEXTURE_2D_ARRAY, colorBuffer);
    glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (integer) {
        if (mono) {
            glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, width, height, nSlices);
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
            glTexParameteriv (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        } else
            glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32UI, width, height, nSlices);
    } else {
        if (mono) {
            glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_R16F, width, height, nSlices);
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
            glTexParameteriv (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        } else
            glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16F, width, height, nSlices);
    }

    GLuint err = glGetError ();
    if (err != GL_NO_ERROR) {
        GLint memAvailable = 0;
        glGetIntegerv (0x9049 /*GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX*/, &memAvailable);
        std::stringstream ss;
        ss << "Available video memory: " << memAvailable << "kb" << std::endl;

        ss << "Required texture queue size: " << width << " x " << height << " x " << nSlices << " x 8 = " << width * height * nSlices * 8 << " bytes" << std::endl;
        ss << "TextureQueue: Error creating texture queue: " << err << std::endl;
        PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
        boost::python::throw_error_already_set ();
        std::cout << "TextureQueue: Error creating texture queue: " << err << std::endl;
    }

    glFramebufferTextureLayer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0, 0);
    buffer = GL_COLOR_ATTACHMENT0;
    err    = glGetError ();
    if (err != GL_NO_ERROR) {
        std::cout << "TextureQueue: Error creating color attachment: " << err << std::endl;
    }

    GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "TextureQueue: Incomplete TextureQueue (";
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";
                break;
        }
        std::cout << ")" << std::endl;
    }

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
#endif
}

TextureQueue::~TextureQueue ()
{
#if 0
    glDeleteFramebuffers (1, &handle);
    if (glGetError () != GL_NO_ERROR) {
        std::cout << "TextureQueue: Error deleting frame buffer." << std::endl;
    }

    glDeleteTextures (1, &colorBuffer);
    if (glGetError () != GL_NO_ERROR) {
        std::cout << "TextureQueue: Error deleting texture." << std::endl;
    }
#endif
}

void TextureQueue::setRenderTarget (GLuint slice)
{
    GVK_BREAK ("unused func");
#if 0
    glBindFramebuffer (GL_FRAMEBUFFER, handle);
    glFramebufferTextureLayer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0, slice);
    glBindFramebuffer (GL_FRAMEBUFFER, handle);

    glViewport (0, 0, width, height);

    glDrawBuffers (1, &buffer);
#endif
}

void TextureQueue::disableRenderTarget ()
{
    GVK_BREAK ("unused func");
#if 0
    GLenum tmpBuff[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers (1, tmpBuff);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
#endif
}

void TextureQueue::clear ()
{
    GVK_BREAK ("unused func");
#if 0
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth (1.0f);
    for (unsigned int i = 0; i < nSlices; i++) {
        setRenderTarget (i);
        glClear (GL_COLOR_BUFFER_BIT);
    }
    disableRenderTarget ();
#endif
}


void TextureQueue::setRenderTargets ()
{
    GVK_BREAK ("unused func");
#if 0
    glBindFramebuffer (GL_FRAMEBUFFER, handle);
    for (unsigned int i = 0; i < nSlices; i++) {
        glFramebufferTextureLayer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorBuffer, 0, i);
    }
    glBindFramebuffer (GL_FRAMEBUFFER, handle);

    glViewport (0, 0, width, height);

    GLenum tmpBuff[] = {GL_COLOR_ATTACHMENT0,
                        GL_COLOR_ATTACHMENT1,
                        GL_COLOR_ATTACHMENT2,
                        GL_COLOR_ATTACHMENT3,
                        GL_COLOR_ATTACHMENT4,
                        GL_COLOR_ATTACHMENT5,
                        GL_COLOR_ATTACHMENT6,
                        GL_COLOR_ATTACHMENT7,
                        GL_COLOR_ATTACHMENT8};
    glDrawBuffers (nSlices, tmpBuff);
#endif
}

void TextureQueue::disableRenderTargets ()
{
    GVK_BREAK ("unused func");
#if 0
    GLenum tmpBuff[] = {GL_COLOR_ATTACHMENT0,
                        GL_COLOR_ATTACHMENT1,
                        GL_COLOR_ATTACHMENT2,
                        GL_COLOR_ATTACHMENT3,
                        GL_COLOR_ATTACHMENT4,
                        GL_COLOR_ATTACHMENT5,
                        GL_COLOR_ATTACHMENT6,
                        GL_COLOR_ATTACHMENT7,
                        GL_COLOR_ATTACHMENT8};
    glDrawBuffers (nSlices, tmpBuff);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
#endif
}
