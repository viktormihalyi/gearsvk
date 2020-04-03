
#include "RandomSequenceBuffer.hpp"

#include <cmath>
#include <iostream>
#include <string>

RandomSequenceBuffer::RandomSequenceBuffer (GLuint width, GLuint height, void* initData)
{
    this->width  = width;
    this->height = height;

    throw std::runtime_error ("DISABLED CODE");
#if 0
	glGenFramebuffers(1, &handle);
	glGenTextures(1, &colorBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);

	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, initData);

	GLenum err = glGetError();
	if(err != GL_NO_ERROR){
		std::cout << "RandomSequenceBuffer: glTexImage2D failed" << std::endl;
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	buffer = GL_COLOR_ATTACHMENT0;
	if(glGetError() != GL_NO_ERROR){
		std::cout << "RandomSequenceBuffer: Error creating color attachment" << std::endl;
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE){
		std::cout << "RandomSequenceBuffer: Incomplete RandomSequenceBuffer (";
		switch(status){
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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

RandomSequenceBuffer::~RandomSequenceBuffer ()
{
    // glDeleteFramebuffers (1, &handle);
}

void RandomSequenceBuffer::setRenderTarget ()
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    glBindFramebuffer (GL_FRAMEBUFFER, handle);

    glViewport (0, 0, width, height);

    glDrawBuffers (1, &buffer);
#endif
}

void RandomSequenceBuffer::disableRenderTarget ()
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    GLenum tmpBuff[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers (1, tmpBuff);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
#endif
}
