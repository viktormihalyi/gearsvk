#include "StimulusWindow.h"
#include <chrono>
#include <ctime>

void StimulusWindow::render ()
{
    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
	makeCurrent();
	//glViewport(0, 0, screenw, screenh);

	//glClearColor(0.f, 0.f, 0.f, 1.f);
	//glClear(GL_COLOR_BUFFER_BIT);
	if (sequenceRenderer->exporting())
		setSwapInterval(0);
	else
		setSwapInterval(sequenceRenderer->getSequence()->frameRateDivisor);

	sequenceRenderer->setScreenResolution(screenw, screenh);

	// render 3 frame in one image if high frequence device used
	size_t channelNum = sequenceRenderer->getSequence()->useHighFreqRender ? 3 : 1;

	for (size_t channelIdx = 0; channelIdx < channelNum; channelIdx++)
	{
		auto start = std::chrono::system_clock::now();
		if (!sequenceRenderer->renderFrame(0, channelIdx))
		{
			quit = true;
			break;
		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsedSeconds = end - start;
		//std::cout << "Length of renderFrame for " << (sequenceRenderer->getSequence()->useOpenCL ? "cl" : "gl") << "fft: " << elapsedSeconds.count() * 1000 << "ms." << std::endl;
	}
	
	swapBuffers();
	// glFinish();

    if (ticker)
        ticker->onBufferSwap ();
}

void StimulusWindow::preRender ()
{
    makeCurrent ();
    //glViewport (0, 0, screenw, screenh);

    sequenceRenderer->preRender ();

    if (sequenceRenderer->getSequence ()->getUsesBusyWaitingThreadForSingals ())
        ticker = sequenceRenderer->startTicker ();
}

void StimulusWindow::postRender ()
{
    if (sequenceRenderer->getSequence ()->getUsesBusyWaitingThreadForSingals ())
        ticker->stop ();
    ticker.reset ();
    sequenceRenderer->reset ();

    if (onHideCallback)
        onHideCallback ();
}