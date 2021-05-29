#include "stdafx.h"
#include "Ticker.h"
#include "SequenceRenderer.h"
#include <iostream>
#ifdef __linux__
	#include <pthread.h>
#endif

void Ticker::start(float tickInterval_s, float frameInterval_s)
{
	ticksPerFrame = (int)(frameInterval_s / tickInterval_s + 0.5);
	ticksToGoInCurrentFrame = 0;
	vsync = false;

	tickInterval = std::chrono::duration_cast< std::chrono::high_resolution_clock::duration > (std::chrono::nanoseconds((long long)(tickInterval_s * 1000000000.0)));

#ifdef _WIN32
	tickerThread = new std::thread(&Ticker::run, this);
	SetThreadPriority( tickerThread->native_handle(), THREAD_PRIORITY_HIGHEST);
#elif __linux__
	pthread_t thId = pthread_self();
    pthread_attr_t thAttr;
    int policy = 0;
    int max_prio_for_policy = 0;

    pthread_attr_init(&thAttr);
    pthread_attr_getschedpolicy(&thAttr, &policy);
    max_prio_for_policy = sched_get_priority_max(policy);


    pthread_setschedprio(thId, max_prio_for_policy);
    pthread_attr_destroy(&thAttr);
#endif
}

void Ticker::stop()
{
	live = false;
	tickerThread->join();
	delete tickerThread;
}

void Ticker::onBufferSwap()
{
//	typedef std::chrono::high_resolution_clock Clock;
//	ticksToGoInCurrentFrame = ticksPerFrame;
//	previousTickTimePoint = Clock::now() - tickInterval;

	vsync = true;
}

void Ticker::run() 
{
	typedef std::chrono::high_resolution_clock Clock;

	auto spikeDuration = std::chrono::high_resolution_clock::duration( (std::chrono::high_resolution_clock::rep)( 100.0) );

	auto previousTickTimePoint = Clock::now();
	while(live)
	{
		auto t = Clock::now();
		auto dt = t - previousTickTimePoint; 
		if(vsync)
		{
			vsync = false;
			if(dt < tickInterval / 2)
				previousTickTimePoint = Clock::now();
			else
				previousTickTimePoint = Clock::now() - tickInterval;
		}
		else if(dt > tickInterval)
		{
			previousTickTimePoint += tickInterval;
			uint iTick = 1;
			auto& signals = sequenceRenderer->tick(iTick);

			//for(auto& signal : signals)
			//{
			//	if( (signal.first != 0 && iTick % signal.first == signal.first-1) || (iTick==1 && signal.first == 0))
			//	{
			//		if(signal.second.clear)
			//			sequenceRenderer->clearSignal(signal.second.channel);
			//		else
			//			sequenceRenderer->raiseSignal(signal.second.channel);
			//	}
			//}
			Stimulus::SignalMap::const_iterator iSignal = signals.find(iTick);
			bool handled = false;
			while(iSignal != signals.end() && iSignal->first == iTick)
			{
				if(iSignal->second.clear)
					sequenceRenderer->clearSignal(iSignal->second.channel);
				else
					sequenceRenderer->raiseSignal(iSignal->second.channel);
				iSignal++;
				handled = true;
			}
			if(!handled)
			{
				Stimulus::SignalMap::const_iterator iSignal = signals.find(0);
				while(iSignal != signals.end() && iSignal->first == 0)
				{
					if(iSignal->second.clear)
						sequenceRenderer->clearSignal(iSignal->second.channel);
					else
						sequenceRenderer->raiseSignal(iSignal->second.channel);
					iSignal++;
				}
			}
		}
	}
}