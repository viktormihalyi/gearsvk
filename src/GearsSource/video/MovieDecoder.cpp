#include "stdafx.h"
#include "MovieDecoder.h"
#include "VideoFrame.h"

#include <cassert>
#include <iostream>
#include <chrono>


extern "C" {
	#include <libswscale/swscale.h>
	#include <libavfilter/avfilter.h>
}

#define VIDEO_QUEUESIZE 40
#define AUDIO_QUEUESIZE 50
#define VIDEO_FRAMES_BUFFERSIZE 5

using namespace std;

MovieDecoder::MovieDecoder()
: m_VideoStream(-1)
, m_pFormatContext(NULL)
, m_pVideoCodecContext(NULL)
, m_pVideoCodec(NULL)
, m_pVideoStream(NULL)
, m_pFrame(NULL)
, m_MaxVideoQueueSize(VIDEO_QUEUESIZE)
, m_pPacketReaderThread(NULL)
, m_Stop(false)
, m_VideoClock(0.0)
, m_Loop(true)
{
}

MovieDecoder::~MovieDecoder()
{
    destroy();
}

void MovieDecoder::destroy()
{
    stop();

    if (m_pFrame)
    {
		av_frame_unref(m_pFrame);
        //av_free(m_pFrame);
        m_pFrame = NULL;
    }

    if (m_pVideoCodecContext)
    {
        avcodec_close(m_pVideoCodecContext);
        m_pVideoCodecContext = NULL;
    }

    if (m_pFormatContext)
    {
#if LIBAVCODEC_VERSION_MAJOR < 53
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = NULL;
#else
        avformat_close_input(&m_pFormatContext);
#endif
    }

}

bool MovieDecoder::initialize(const string& filename)
{
    bool ok = true;

    av_register_all();
    avcodec_register_all();
    
#if LIBAVCODEC_VERSION_MAJOR < 53
    if (av_open_input_file(&m_pFormatContext, filename.c_str(), NULL, 0, NULL) != 0)
#else
    if (avformat_open_input(&m_pFormatContext, filename.c_str(), NULL, NULL) != 0)
#endif
    {
        cerr << "Could not open input file: " << filename.c_str() << endl;
        return false;
    }

#if LIBAVCODEC_VERSION_MAJOR < 53
    if (av_find_stream_info(m_pFormatContext) < 0)
#else
    if (avformat_find_stream_info(m_pFormatContext, NULL) < 0)
#endif
    {
        cerr << "could not find stream information" << endl;
        return false;
    }

#ifdef _DEBUG
    //dump_format(m_pFormatContext, 0, filename.c_str(), false);
    av_log_set_level(AV_LOG_DEBUG);
#endif

    ok = ok && initializeVideo();

    return ok;
}

bool MovieDecoder::initializeVideo()
{
    for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
    {
#if LIBAVCODEC_VERSION_MAJOR < 53
        if (m_pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
#else
        if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
#endif
        {
            m_pVideoStream = m_pFormatContext->streams[i];
            m_VideoStream = i;
            break;
        }
    }

    if (m_VideoStream == -1)
    {
        cerr << "Could not find video stream" << endl;
        return false;
    }

    m_pVideoCodecContext = m_pFormatContext->streams[m_VideoStream]->codec;
    m_pVideoCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);

    if (m_pVideoCodec == NULL)
    {
        // set to NULL, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = NULL;
        cerr << "Video Codec not found" << endl;
        return false;
    }

    m_pVideoCodecContext->workaround_bugs = 1;
    m_pFormatContext->flags |= AVFMT_FLAG_GENPTS;

#if LIBAVCODEC_VERSION_MAJOR < 53
    if (avcodec_open(m_pVideoCodecContext, m_pVideoCodec) < 0)
#else
    if (avcodec_open2(m_pVideoCodecContext, m_pVideoCodec, NULL) < 0)
#endif
    {
        cerr << "Could not open video codec" << endl;
        return false;
    }

    //m_pFrame = avcodec_alloc_frame();
	m_pFrame = av_frame_alloc();
	
    return true;
}


int MovieDecoder::getFrameHeight()
{
    return m_pVideoCodecContext ? m_pVideoCodecContext->height : -1;
}

int MovieDecoder::getFrameWidth()
{
    return m_pVideoCodecContext ? m_pVideoCodecContext->width : -1;
}

double MovieDecoder::getVideoClock()
{
    return m_VideoClock;
}

float MovieDecoder::getProgress()
{
    if (m_pFormatContext)
    {
        return static_cast<float>(m_VideoClock / (m_pFormatContext->duration / AV_TIME_BASE));
    }

    return 0.f;
}

float MovieDecoder::getDuration()
{
    return static_cast<float>(m_pFormatContext->duration / AV_TIME_BASE);
}

void MovieDecoder::seek(int offset)
{
    ::int64_t timestamp = (::int64_t)((m_VideoClock * AV_TIME_BASE) + (AV_TIME_BASE * offset));
    int flags = offset < 0 ? AVSEEK_FLAG_BACKWARD : 0;

    if (timestamp < 0)
    {
        timestamp = 0;
    }

    cout << "av_gettime: " << timestamp << " timebase " << AV_TIME_BASE << " " << flags << endl;
    int ret = av_seek_frame(m_pFormatContext, -1, timestamp, flags);

    if (ret >= 0)
    {
        m_VideoClock = (double) timestamp / AV_TIME_BASE;
        std::lock_guard<std::mutex> videoLock(m_VideoQueueMutex);

        clearQueue(m_VideoQueue);

        avcodec_flush_buffers(m_pFormatContext->streams[m_VideoStream]->codec);
    }
    else
    {
        cerr << "Error seeking to position: " << timestamp << endl;
    }
}

bool MovieDecoder::decodeVideoFrame(VideoFrame& frame)
{
    AVPacket    packet;
    bool        frameDecoded = false;

    do
    {
        if (!popVideoPacket(&packet))
        {
            return false;
        }
        frameDecoded = decodeVideoPacket(packet);
    } while (!frameDecoded);

	// TODO deinterlace with yadif
    if (m_pFrame->interlaced_frame)
    {
		bool interlaced = true;
//		   /* Suppose I have a AVCodecContext pointer somewhere and I'm
//      able to get the next decoded frame anytime */
//
//    // Register all built-in filters
//    avfilter_register_all();
//
//    // Find the yadif filter
//    AVFilter *yadif_filter = avfilter_get_by_name("yadif");
//
//    AVFilterContext *filter_ctx;
//
//    // Create the filter context with yadif filter
//    avfilter_open(&filter_ctx, yadif_filter, NULL);
//
//    // Init the yadif context with "1:-1" option
//    avfilter_init_filter(filter_ctx, "1:-1", NULL);
//
//    /* Q1: Here I probably need to teach the filter context how to get the
//       next frame from the AVCodecContext using AVFilter input/output
//       pads (unclear). But how exactly? */
//
//    for (;;)
//    {
//        /* Q2: Here I need to: 1) tell the filter context to get the
//next frame from its
//           source/input/output pads (unclear) and apply its logic; 2) Read the
//           filtered frames (should be 2 per frame read, because I need
//a deinterlace
//           with 1 frame per filed). How I do 1) and 2) exactly? */
//    }
//        avpicture_deinterlace((AVPicture*) m_pFrame, (AVPicture*) m_pFrame,
//                              m_pVideoCodecContext->pix_fmt,
//                              m_pVideoCodecContext->width,
//                              m_pVideoCodecContext->height);
    }


    if (m_pVideoCodecContext->pix_fmt != PIX_FMT_YUV420P)
    {
        convertVideoFrame(PIX_FMT_YUV420P);
    }

    m_VideoClock = packet.dts * av_q2d(m_pVideoStream->time_base);

    frame.setWidth(getFrameWidth());
    frame.setHeight(getFrameHeight());

    frame.storeYPlane(m_pFrame->data[0], m_pFrame->linesize[0]);
    frame.storeUPlane(m_pFrame->data[1], m_pFrame->linesize[1]);
    frame.storeVPlane(m_pFrame->data[2], m_pFrame->linesize[2]);
    frame.setPts(m_VideoClock);

    return frameDecoded;
}

void MovieDecoder::convertVideoFrame(PixelFormat format)
{
    SwsContext* scaleContext = sws_getContext(m_pVideoCodecContext->width, m_pVideoCodecContext->height,
                                              m_pVideoCodecContext->pix_fmt, m_pVideoCodecContext->width, m_pVideoCodecContext->height,
                                              format, 0, NULL, NULL, NULL);

    if (NULL == scaleContext)
    {
        throw logic_error("Failed to create resize context");
    }

    AVFrame* convertedFrame = NULL;
    createAVFrame(&convertedFrame, m_pVideoCodecContext->width, m_pVideoCodecContext->height, format);

    sws_scale(scaleContext, m_pFrame->data, m_pFrame->linesize, 0, m_pVideoCodecContext->height,
              convertedFrame->data, convertedFrame->linesize);
    sws_freeContext(scaleContext);

    //av_free(m_pFrame);
	av_frame_unref(m_pFrame);
    m_pFrame = convertedFrame;
}

void MovieDecoder::createAVFrame(AVFrame** avFrame, int width, int height, PixelFormat format)
{
//    *avFrame = avcodec_alloc_frame();
	*avFrame = av_frame_alloc();

    int numBytes = avpicture_get_size(format, width, height);
    uint8_t* pBuffer = new uint8_t[numBytes];

    avpicture_fill((AVPicture*) *avFrame, pBuffer, format, width, height);
}

bool MovieDecoder::decodeVideoPacket(AVPacket& packet)
{
    int frameFinished;
    std::lock_guard<std::mutex> lock(m_DecodeVideoMutex);
    int bytesDecoded = avcodec_decode_video2(m_pVideoCodecContext, m_pFrame, &frameFinished, &packet);
    av_free_packet(&packet);
    if (bytesDecoded < 0)
    {
        cerr << "Failed to decode video frame: bytesDecoded < 0" << endl;
    }

    return frameFinished > 0;
}

void MovieDecoder::readPackets()
{
    AVPacket    packet;
    bool        framesAvailable = true;

    while(framesAvailable && !m_Stop)
    {
        if ((int) m_VideoQueue.size() >= m_MaxVideoQueueSize)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            
        }
        else if (av_read_frame(m_pFormatContext, &packet) >= 0)
        {
            if (packet.stream_index == m_VideoStream)
            {
                queueVideoPacket(&packet);
            }
            else
            {
                av_free_packet(&packet);
            }
        }
        else
        {
			if(m_Loop)
  				seek(-1000);
			else
	            framesAvailable = false;
        }
    }
}

void MovieDecoder::start()
{
    m_Stop = false;
    if (!m_pPacketReaderThread)
    {
        m_pPacketReaderThread = new thread(bind(&MovieDecoder::readPackets, this));
    }
}

void MovieDecoder::stop()
{
    m_VideoClock = 0;
    m_Stop = true;
    if (m_pPacketReaderThread)
    {
        m_pPacketReaderThread->join();
        delete m_pPacketReaderThread;
        m_pPacketReaderThread = NULL;
    }

    clearQueue(m_VideoQueue);
}

bool MovieDecoder::queueVideoPacket(AVPacket* packet)
{
    std::lock_guard<std::mutex> lock(m_VideoQueueMutex);
    return queuePacket(m_VideoQueue, packet);
}

bool MovieDecoder::queuePacket(queue<AVPacket>& packetQueue, AVPacket* packet)
{
    if (av_dup_packet(packet) < 0)
    {
        return false;
    }
    packetQueue.push(*packet);

    return true;
}

bool MovieDecoder::popPacket(queue<AVPacket>& packetQueue, AVPacket* packet)
{
    if (packetQueue.empty())
    {
        return false;
    }

    *packet = packetQueue.front();
    packetQueue.pop();

    return true;
}

void MovieDecoder::clearQueue(std::queue<AVPacket>& packetQueue)
{
    while(!packetQueue.empty())
    {
        packetQueue.pop();
    }
}

bool MovieDecoder::popVideoPacket(AVPacket* packet)
{
    std::lock_guard<std::mutex> lock(m_VideoQueueMutex);
    return popPacket(m_VideoQueue, packet);
}

void MovieDecoder::bufferPackets()
{
	bool full = false;
	while(!full)
	{
		{
		std::lock_guard<std::mutex> lock(m_VideoQueueMutex);
		full = ( m_VideoQueue.size() == m_MaxVideoQueueSize );
		}
		Sleep(100);
	}
}

void MovieDecoder::printQueueSize()
{
	std::cout << "Video Q size: " << m_VideoQueue.size() << std::endl;
}