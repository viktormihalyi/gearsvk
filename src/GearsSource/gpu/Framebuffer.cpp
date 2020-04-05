
#include "Framebuffer.hpp"

//#include <boost/python.hpp>
#include <cmath>
#include <iostream>
#include <string>

// Calculates log2 of number.
float Log2 (float n)
{
    // log(n)/log(2) is log2.
    return log (n) / log (2.0f);
}

Framebuffer::Framebuffer (GLuint width, GLuint height, GLuint planes, GLenum format, bool genMipMaps)
{
    this->width  = width;
    this->height = height;
    this->planes = planes;
    hasMipMaps   = genMipMaps;

    if (width != height)
        hasMipMaps = false;
    else {
        float fnumMips = Log2 ((float)width);
        if (fmod (fnumMips, 1.0f) != 0)
            hasMipMaps = false;
        else
            numMips = (int)(fnumMips + 1);
    }

    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    if (genMipMaps && !hasMipMaps) {
        std::cerr << "Framebuffer: Generation of mipmaps failed. The texture is not square, or not power of 2!";
        PyErr_SetString (PyExc_RuntimeError, "Framebuffer: Generation of mipmaps failed. The texture is not square, or not power of 2!");
        boost::python::throw_error_already_set ();
    }

    buffers = new GLenum[planes];

    glGenFramebuffers (1, &handle);
    colorBuffer = new GLuint[planes];
    glGenTextures (planes, colorBuffer);
    glBindFramebuffer (GL_FRAMEBUFFER, handle);

    for (unsigned int i = 0; i < planes; ++i) {
        glBindTexture (GL_TEXTURE_2D, colorBuffer[i]);
        if (hasMipMaps) {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        } else {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (format == GL_RGBA32F)
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        else if (format == GL_RGBA16F)
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        else if (format == GL_RGBA8)
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        else if (format == GL_R8)
            glTexImage2D (GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);
        buffers[i] = GL_COLOR_ATTACHMENT0 + i;
        if (glGetError () != GL_NO_ERROR) {
            std::cerr << "Framebuffer: Error creating color attachment.";
            PyErr_SetString (PyExc_RuntimeError, "Framebuffer: Error creating color attachment.");
            boost::python::throw_error_already_set ();
        }

        if (hasMipMaps) {
            int size = width;
            for (int l = 1; l < numMips; l++) {
                size = size / 2;
                if (GL_RGBA32F)
                    glTexImage2D (GL_TEXTURE_2D, l, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
                else
                    glTexImage2D (GL_TEXTURE_2D, l, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            }
        }
    }

    glGenRenderbuffers (1, &depthBuffer);
    glBindRenderbuffer (GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer (GL_RENDERBUFFER, 0);

    glBindFramebuffer (GL_FRAMEBUFFER, handle);
    glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    if (glGetError () != GL_NO_ERROR) {
        std::cerr << "Framebuffer: Error creating depth attachment.\n";
        PyErr_SetString (PyExc_RuntimeError, "Framebuffer: Error creating depth attachment.");
        boost::python::throw_error_already_set ();
    }

    GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "Incomplete framebuffer (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT).\n";
                PyErr_SetString (PyExc_RuntimeError, "Incomplete framebuffer (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT).");
                boost::python::throw_error_already_set ();
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "Incomplete framebuffer (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT).\n";
                PyErr_SetString (PyExc_RuntimeError, "Incomplete framebuffer (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT).");
                boost::python::throw_error_already_set ();
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "Incomplete framebuffer (GL_FRAMEBUFFER_UNSUPPORTED).\n";
                PyErr_SetString (PyExc_RuntimeError, "Incomplete framebuffer (GL_FRAMEBUFFER_UNSUPPORTED).");
                boost::python::throw_error_already_set ();
                break;
        }
    }

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
#endif
}

Framebuffer::~Framebuffer ()
{
#if 0
    glDeleteFramebuffers (1, &handle);
    glDeleteRenderbuffers (1, &depthBuffer);
#endif
}

void Framebuffer::setRenderTarget (int mipLevel)
{
    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    glBindFramebuffer (GL_FRAMEBUFFER, handle);

    if (mipLevel >= 0 && hasMipMaps) {
        for (unsigned int i = 0; i < planes; i++) {
            glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], mipLevel);
        }
        int texsize = (int)pow (2, numMips - 1 - mipLevel);
        glViewport (0, 0, texsize, texsize);
    } else
        glViewport (0, 0, width, height);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
    glBindRenderbuffer (GL_RENDERBUFFER, depthBuffer);

    glDrawBuffers (planes, buffers);
#endif
}

void Framebuffer::disableRenderTarget ()
{
    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    GLenum tmpBuff[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers (1, tmpBuff);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
#endif
}

void Framebuffer::clear ()
{
    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth (1.0f);
    setRenderTarget ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    disableRenderTarget ();
#endif
}
