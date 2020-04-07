#include "stdafx.h"
#include "SequenceRenderer.h"
#pragma region OpenGL
#include <GL/glew.h>
#pragma endregion includes for GLEW
#include <fstream>
#include <sstream>

#include <ctime>
#include <limits>
#include "SpatialFilterRenderer.h"

SpatialFilterRenderer::SpatialFilterRenderer(SequenceRenderer::P sequenceRenderer, ShaderManager::P shaderManager, KernelManager::P _kernelManager, SpatialFilter::P _spatialFilter):
	sequenceRenderer(sequenceRenderer), kernelManager(_kernelManager), shaderManager(shaderManager), spatialFilter(_spatialFilter)
{
	channelMode = sequenceRenderer->getSequence()->isMonochrome() ? FFTChannelMode::Monochrome : FFTChannelMode::Multichrome;
	renderQuad = [this]() 
	{
		this->sequenceRenderer->getNothing()->renderQuad();
	};

	if(spatialFilter)
	{
		spatialKernelId = kernelManager->getKernel(spatialFilter);
		spatialDomainConvolutionShader = shaderManager->loadShader(spatialFilter->spatialDomainConvolutionShaderSource);
	}
}

void SpatialFilterRenderer::initFirstFrames(std::function<void(int)> stim)
{
}

void SpatialFilterRenderer::renderFrame(std::function<void(int)> renderStimulus)
{
	renderStim = renderStimulus;
	if(spatialFilter->useFft)
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		fftConvolution();

		if(sequenceRenderer->sequence->getMaxTemporalProcessingStateCount() > 0)
		{
			sequenceRenderer->nextTemporalProcessingState->setRenderTargets();
			glViewport(
				0,
				0,
				sequenceRenderer->sequence->fieldWidth_px,
				sequenceRenderer->sequence->fieldHeight_px);

		}
		else if(sequenceRenderer->sequence->getMaxMemoryLength() > 0)
		{
			sequenceRenderer->textureQueue->setRenderTarget(sequenceRenderer->currentSlice);
			glViewport(
				0,
				0,
				sequenceRenderer->sequence->fieldWidth_px,
				sequenceRenderer->sequence->fieldHeight_px);

		}
		else
			glViewport(sequenceRenderer->sequence->fieldLeft_px, sequenceRenderer->sequence->fieldBottom_px, sequenceRenderer->sequence->fieldWidth_px, sequenceRenderer->sequence->fieldHeight_px);

		copyShader->enable();

		//		copyShader->bindUniformInt2("offset", 
		////			sequenceRenderer->sequence->fftWidth_px * sequenceRenderer->sequence->getMaxKernelWidth_um() / sequenceRenderer->sequence->getSpatialFilteredFieldWidth_um() / 2,
		////			sequenceRenderer->sequence->fftHeight_px * sequenceRenderer->sequence->getMaxKernelHeight_um() / sequenceRenderer->sequence->getSpatialFilteredFieldHeight_um() / 2 );
		////			-512,
		////			-384 
		//			-700,
		//			-1000
		//			);

		copyShader->bindUniformInt2("fftSize",
			sequenceRenderer->sequence->fftWidth_px,
			sequenceRenderer->sequence->fftHeight_px);
		copyShader->bindUniformInt2("offset",
			(int)(sequenceRenderer->sequence->fftWidth_px * (1.0 - sequenceRenderer->sequence->fieldWidth_um / sequenceRenderer->sequence->getSpatialFilteredFieldWidth_um() / 2)),
			(int)(sequenceRenderer->sequence->fftHeight_px * (1.0 - sequenceRenderer->sequence->fieldHeight_um / sequenceRenderer->sequence->getSpatialFilteredFieldHeight_um() / 2))
		);

		copyShader->bindUniformFloat("pixelArea", sequenceRenderer->sequence->getSpatialFilteredFieldWidth_um() * sequenceRenderer->sequence->getSpatialFilteredFieldHeight_um()
			/ (float)sequenceRenderer->sequence->fftWidth_px / (float)sequenceRenderer->sequence->fftHeight_px);

		double que = (double)sequenceRenderer->sequence->fftWidth_px * sequenceRenderer->sequence->fieldWidth_um /
			(sequenceRenderer->sequence->fieldWidth_px * sequenceRenderer->sequence->getSpatialFilteredFieldWidth_um());
		double vad =
			(double)sequenceRenderer->sequence->fftHeight_px * sequenceRenderer->sequence->fieldHeight_um
			/ (sequenceRenderer->sequence->getSpatialFilteredFieldHeight_um() * sequenceRenderer->sequence->fieldHeight_px);
		copyShader->bindUniformFloat2("pixelRatio",
			(float)sequenceRenderer->sequence->fftWidth_px * sequenceRenderer->sequence->fieldWidth_um /
			(sequenceRenderer->sequence->fieldWidth_px * sequenceRenderer->sequence->getSpatialFilteredFieldWidth_um()),
			(float)sequenceRenderer->sequence->fftHeight_px * sequenceRenderer->sequence->fieldHeight_um
			/ (sequenceRenderer->sequence->getSpatialFilteredFieldHeight_um() * sequenceRenderer->sequence->fieldHeight_px));


		bindTexture();

		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		sequenceRenderer->getNothing()->renderQuad();

		copyShader->disable();

		prepareNext();

		if(sequenceRenderer->sequence->getMaxTemporalProcessingStateCount() > 0)
		{
			sequenceRenderer->nextTemporalProcessingState->disableRenderTargets();
		}
		else if(sequenceRenderer->sequence->getMaxMemoryLength() > 0)
		{
			sequenceRenderer->textureQueue->disableRenderTarget();
		}

		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		normalConvolution();
	}
}

void SpatialFilterRenderer::normalConvolution()
{
	sequenceRenderer->spatialDomainFilteringBuffers[0]->setRenderTarget(0);
	renderStim(0);
	sequenceRenderer->spatialDomainFilteringBuffers[0]->disableRenderTarget();

	///////////////
	glViewport(
		0,
		0,
		sequenceRenderer->sequence->fieldWidth_px,
		sequenceRenderer->sequence->fieldHeight_px);

	if(spatialFilter->separable)
	{
		sequenceRenderer->spatialDomainFilteringBuffers[1]->setRenderTarget(0);
		spatialDomainConvolutionShader->enable();



		spatialDomainConvolutionShader->bindUniformTexture("original", sequenceRenderer->spatialDomainFilteringBuffers[0]->getColorBuffer(0), 0);
		spatialDomainConvolutionShader->bindUniformTexture("kernel", spatialKernelId, 1);
		spatialDomainConvolutionShader->bindUniformFloat2("patternSizeOnRetina", sequenceRenderer->sequence->fieldWidth_um, sequenceRenderer->sequence->fieldHeight_um);
		spatialDomainConvolutionShader->bindUniformFloat2("kernelSizeOnRetina", spatialFilter->width_um, spatialFilter->height_um);
		spatialDomainConvolutionShader->bindUniformFloat2("step", 0, spatialFilter->height_um / 17.0f);
		spatialDomainConvolutionShader->bindUniformBool("combine", false);

		sequenceRenderer->getNothing()->renderQuad();
		spatialDomainConvolutionShader->disable();

		sequenceRenderer->spatialDomainFilteringBuffers[1]->disableRenderTarget();
	}

	///////////////

	if(sequenceRenderer->sequence->getMaxTemporalProcessingStateCount() > 0)
	{
		sequenceRenderer->nextTemporalProcessingState->setRenderTargets();
	}
	else if(sequenceRenderer->sequence->getMaxMemoryLength() > 0)
	{
		sequenceRenderer->textureQueue->setRenderTarget(sequenceRenderer->currentSlice);
	}

	spatialDomainConvolutionShader->enable();

	glViewport(
		0,
		0,
		sequenceRenderer->sequence->fieldWidth_px,
		sequenceRenderer->sequence->fieldHeight_px);
	//glViewport( sequenceRenderer->sequence->fieldLeft_px, sequenceRenderer->sequence->fieldBottom_px, sequenceRenderer->sequence->fieldWidth_px, sequenceRenderer->sequence->fieldHeight_px);

	if(spatialFilter->separable)
		spatialDomainConvolutionShader->bindUniformTexture("original", sequenceRenderer->spatialDomainFilteringBuffers[1]->getColorBuffer(0), 0);
	else
		spatialDomainConvolutionShader->bindUniformTexture("original", sequenceRenderer->spatialDomainFilteringBuffers[0]->getColorBuffer(0), 0);
	spatialDomainConvolutionShader->bindUniformTexture("kernel", spatialKernelId, 1);
	spatialDomainConvolutionShader->bindUniformFloat2("patternSizeOnRetina", sequenceRenderer->sequence->fieldWidth_um, sequenceRenderer->sequence->fieldHeight_um);
	spatialDomainConvolutionShader->bindUniformFloat2("kernelSizeOnRetina", spatialFilter->width_um, spatialFilter->height_um);
	spatialDomainConvolutionShader->bindUniformFloat2("step", spatialFilter->width_um / 17.0f, 0.f);
	spatialDomainConvolutionShader->bindUniformBool("combine", true);
	sequenceRenderer->getNothing()->renderQuad();
	spatialDomainConvolutionShader->disable();

	if(sequenceRenderer->sequence->getMaxTemporalProcessingStateCount() > 0)
	{
		sequenceRenderer->nextTemporalProcessingState->disableRenderTargets();
	}
	else if(sequenceRenderer->sequence->getMaxMemoryLength() > 0)
	{
		sequenceRenderer->textureQueue->disableRenderTarget();
	}
}

