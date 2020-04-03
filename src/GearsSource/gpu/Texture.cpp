#include <iostream>

#include "Texture.hpp"
#include <sstream>
//#include <boost/python.hpp>

Texture2D::Texture2D ()
{
}

Texture2D::~Texture2D ()
{
}

void Texture2D::initialize (GLuint width, GLuint height, GLuint bpp)
{
    this->width  = width;
    this->height = height;
    this->bpp    = bpp;

#if 0
    glGenTextures (1, &handle);
    glBindTexture (GL_TEXTURE_2D, handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);

    glBindTexture (GL_TEXTURE_2D, 0);
#endif
}

void Texture2D::setData (const float* data)
{
    //glBindTexture (GL_TEXTURE_2D, handle);
    //glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
}

void Texture2D::setData (const float* data, GLuint level)
{
    //glBindTexture (GL_TEXTURE_2D, handle);
    //glTexImage2D (GL_TEXTURE_2D, level, GL_RGBA32F, width >> level, height >> level, 0, GL_RGBA, GL_FLOAT, data);
}

void Texture2D::initializeRG (GLuint width, GLuint height, GLuint bpp)
{
    this->width  = width;
    this->height = height;
    this->bpp    = bpp;

#if 0
    glGenTextures (1, &handle);
    glBindTexture (GL_TEXTURE_2D, handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0);

    glBindTexture (GL_TEXTURE_2D, 0);
#endif
}

void Texture2D::setDataRG (const float* data)
{
    //glBindTexture (GL_TEXTURE_2D, handle);
    //glTexImage2D (GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, data);
}

void Texture2D::initializeGrey (GLuint width, GLuint height, GLuint bpp)
{
    this->width  = width;
    this->height = height;
    this->bpp    = bpp;

    throw std::runtime_error ("DISABLED CODE");
#if 0
    glGenTextures (1, &handle);
    glBindTexture (GL_TEXTURE_2D, handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //  glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8I, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    glBindTexture (GL_TEXTURE_2D, 0);
#endif
}

void Texture2D::setDataGrey (const unsigned char* data)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    glBindTexture (GL_TEXTURE_2D, handle);
    //  glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
#endif
}

void Texture2D::initialize8bit (GLuint width, GLuint height, GLuint bpp)
{
    this->width  = width;
    this->height = height;
    this->bpp    = bpp;

    throw std::runtime_error ("DISABLED CODE");
#if 0
    glGenTextures (1, &handle);
    glBindTexture (GL_TEXTURE_2D, handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE8, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    glBindTexture (GL_TEXTURE_2D, 0);
#endif
}

void Texture2D::setData8bit (const unsigned char* data)
{
    throw std::runtime_error ("DISABLED CODE");
    //glBindTexture (GL_TEXTURE_2D, handle);
    //glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE8, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
}

void Texture2D::loadFromFile (std::string fileName)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    ilInit ();
    ILuint devilError = ilGetError ();
    if (IL_NO_ERROR != devilError) {
        std::stringstream ss;
        ss << iluErrorString (devilError) << std::endl;
        std::cerr << ss.str ();
        PyErr_SetString (PyExc_RuntimeError, ss.str ().c_str ());
        boost::python::throw_error_already_set ();
    }

    ILuint imageHandle;
    ilGenImages (1, &imageHandle);
    ilBindImage (imageHandle);
    ILboolean ret = ilLoadImage (fileName.c_str ());
    if (!ret) {
        std::stringstream ss;
        ss << "Could not load texture file: " << fileName << std::endl;
        std::cerr << ss.str ();
        PyErr_SetString (PyExc_RuntimeError, ss.str ().c_str ());
        boost::python::throw_error_already_set ();
    }
    if (IL_NO_ERROR != devilError) {
        std::stringstream ss;
        ss << iluErrorString (devilError) << std::endl;
        std::cerr << ss.str ();
        PyErr_SetString (PyExc_RuntimeError, ss.str ().c_str ());
        boost::python::throw_error_already_set ();
    }

    width  = (unsigned int)ilGetInteger (IL_IMAGE_WIDTH);
    height = (unsigned int)ilGetInteger (IL_IMAGE_HEIGHT);
    bpp    = (unsigned int)ilGetInteger (IL_IMAGE_BITS_PER_PIXEL);

    ILuint dataSize = ilGetInteger (IL_IMAGE_SIZE_OF_DATA);

    initialize (width, height, bpp);

    float* buffer = new float[width * height * 4];
    ilCopyPixels (0, 0, 0, width, height, 1, IL_RGBA, IL_FLOAT, buffer);

    setData (buffer);
    glGenerateTextureMipmapEXT (handle, GL_TEXTURE_2D);


    glBindTexture (GL_TEXTURE_2D, handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture (GL_TEXTURE_2D, 0);


    ilDeleteImages (1, &imageHandle);
#endif
}

GLuint Texture2D::getTextureHandle ()
{
    throw std::runtime_error ("DISABLED CODE");
    //return handle;
}


Texture1D::Texture1D ()
{
}

Texture1D::~Texture1D ()
{
}

void Texture1D::initialize (GLuint width)
{
    throw std::runtime_error ("DISABLED CODE");
    this->width = width;

#if 0
    glGenTextures (1, &handle);
    glBindTexture (GL_TEXTURE_1D, handle);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D (GL_TEXTURE_1D, 0, GL_R32F, width, 0, GL_RED, GL_FLOAT, 0);

    glBindTexture (GL_TEXTURE_1D, 0);
#endif
}

void Texture1D::setData (const float* data)
{
    throw std::runtime_error ("DISABLED CODE");
    //glBindTexture (GL_TEXTURE_1D, handle);
    //glTexImage1D (GL_TEXTURE_1D, 0, GL_R32F, width, 0, GL_RED, GL_FLOAT, data);
}

GLuint Texture1D::getTextureHandle ()
{
    return handle;
}