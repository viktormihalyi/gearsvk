#include "glSpatialFilterRenderer.h"
#include <stdafx.h>

GLSpatialFilterRenderer::GLSpatialFilterRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, ShaderManager::P shaderManager, KernelManager::P _kernelManager, SpatialFilter::P _spatialFilter, unsigned int width, unsigned int height)
    : SpatialFilterRenderer (sequenceRenderer, shaderManager, _kernelManager, _spatialFilter)
    , fft_rg (width, height)
    , fft_ba (width, height)
    , ifft_rg (width, height, 0, true, true)
    , ifft_ba (width, height, 0, true, true)
{
    convolutionShader = shaderManager->loadShader (R"GLSLC0D3(
			#version 450
			#extension GL_ARB_texture_rectangle : enable
			precision highp float;
			uniform sampler2DRect kernel;
			uniform sampler2DRect stim;
//			uniform ivec2 fftSize;
//			uniform bool showFft;
			in vec2 fTexCoord;
			out vec4 outcolor;
//			void main() { vec4 k =  texture2DRect(kernel, (ivec2(gl_FragCoord.xy) + fftSize/2)%fftSize);
	void main() {
				vec4 k =  texture2DRect(kernel, gl_FragCoord.xy);
//				vec4 k =  vec4(1, 0, 1, 0);
//				vec4 s =  vec4(1, 0, 1, 0);
				vec4 s =  texture2DRect(stim, gl_FragCoord.xy );
				outcolor = vec4(k.x*s.x - k.y*s.y, k.x*s.y + k.y*s.x, k.z*s.z - k.w*s.w, k.z*s.w + k.w*s.z);
//				int cq = (int(gl_FragCoord.x) % 2) ^ (int(gl_FragCoord.y) % 2);
//				if(showFft && (cq == 1))
//					outcolor = -outcolor;

//		 		outcolor = vec4(s.x, s.y, s.z, s.w);
//		 		outcolor = vec4(k.x, k.y, k.z, k.w);
//				outcolor = vec4(1, 1, 0, 0);
			}
		)GLSLC0D3");

    copyShader = shaderManager->loadShader (R"GLSLC0D3(
			#version 450
	    	#extension GL_ARB_texture_rectangle : enable
			precision highp float;
			uniform ivec2 offset;
			uniform ivec2 fftSize;
			uniform vec2 pixelRatio;
			uniform float pixelArea;
			uniform sampler2DRect srcrg;
			uniform sampler2DRect srcba;
			uniform bool clFFT;
			in vec2 fTexCoord;
			out vec4 outcolor;
			void main() {
				vec2 uv = gl_FragCoord.xy * pixelRatio;
				uv = mod(uv + offset, vec2(fftSize)) + vec2(1, 1);
				outcolor = vec4(texture2DRect(srcrg, uv).xz, texture2DRect(srcba, uv).xz) * pixelArea;
				// vec2 tc = (uv + vec2(2, 2)) * textureSize(src) ;
				// int cq = (int(tc.x) % 2) ^ (int(tc.y) % 2);
				// if(cq == 0) outcolor = -outcolor;
			}
		)GLSLC0D3");
}

void GLSpatialFilterRenderer::fftConvolution ()
{
    //// Convolution for rg channels
    // Fourier transformation
    spatialFilter->fftSwizzleMask = 0x00020000;
    fft_rg.set_input (renderStim);
    fft_rg.redraw_input ();
    if (!spatialFilter->stimulusGivenInFrequencyDomain)
        fft_rg.do_fft ();

    unsigned int freqTexId[2];
    freqTexId[0] = fft_rg.get_fullTex ();

    // Convolute in frequency space
    ifft_rg.set_input ([&] (int) {
        convolutionShader->enable ();
        //convolutionShader->bindUniformBool( "showFft", spatialFilter->showFft );
        convolutionShader->bindUniformTextureRect ("kernel", spatialKernelId, 0);
        convolutionShader->bindUniformTextureRect ("stim", freqTexId[0], 1);
        /*convolutionShader->bindUniformInt2( "fftSize",
		sequenceRenderer->sequence->fftWidth_px,
		sequenceRenderer->sequence->fftHeight_px );*/
        renderQuad ();
        convolutionShader->disable ();
    });

    if (channelMode == FFTChannelMode::Multichrome) {
        //// Convolution for ba channels
        // Fourier transformation
        spatialFilter->fftSwizzleMask = 0x00000406;
        fft_ba.set_input (renderStim);
        fft_ba.redraw_input ();
        if (!spatialFilter->stimulusGivenInFrequencyDomain)
            fft_ba.do_fft ();

        freqTexId[1] = fft_ba.get_fullTex ();

        // Convolute in frequency space
        ifft_ba.set_input ([&] (int) {
            convolutionShader->enable ();
            //convolutionShader->bindUniformBool( "showFft", spatialFilter->showFft );
            convolutionShader->bindUniformTextureRect ("kernel", spatialKernelId, 0);
            convolutionShader->bindUniformTextureRect ("stim", freqTexId[1], 1);
            /*convolutionShader->bindUniformInt2( "fftSize",
			sequenceRenderer->sequence->fftWidth_px,
			sequenceRenderer->sequence->fftHeight_px );*/
            renderQuad ();
            convolutionShader->disable ();
        });
    }

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    // Inverse Fourier transformation
    ifft_rg.redraw_input ();
    if (!spatialFilter->showFft)
        ifft_rg.do_fft ();

    if (channelMode == FFTChannelMode::Multichrome) {
        ifft_ba.redraw_input ();
        if (!spatialFilter->showFft)
            ifft_ba.do_fft ();
    }
}

void GLSpatialFilterRenderer::bindTexture ()
{
    copyShader->bindUniformTextureRect ("srcrg", ifft_rg.get_fullTex (), 0);
    if (channelMode == FFTChannelMode::Multichrome)
        copyShader->bindUniformTextureRect ("srcba", ifft_ba.get_fullTex (), 1);
}