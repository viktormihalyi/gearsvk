#pragma once

#ifdef WIN32
    typedef signed char  int8_t;
    typedef signed short int16_t;
    typedef signed int   int32_t;
    typedef unsigned char  uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned int   uint32_t;
    typedef signed long long   int64_t;
    typedef unsigned long long uint64_t;
    typedef signed char int_fast8_t;
    typedef signed int  int_fast16_t;
    typedef signed int  int_fast32_t;
    typedef unsigned char uint_fast8_t;
    typedef unsigned int  uint_fast16_t;
    typedef unsigned int  uint_fast32_t;
    typedef uint64_t      uint_fast64_t;
#else
    #include <inttypes.h>
#endif

#include <string>
#include <queue>

extern "C" {
#include <libavcodec/avcodec.h>
#pragma warning( push )  
#pragma warning( disable : 4244 )  
#include <libavformat/avformat.h>
#pragma warning( pop )
}

#include <thread>
#include <mutex>

#include "VideoFrame.h"


class MovieDecoder
{
public:
    MovieDecoder();
    ~MovieDecoder();

    bool initialize(const std::string& filename);
    void destroy();
    bool decodeVideoFrame(VideoFrame& videoFrame);
    void seek(int offset);
    void start();
    void stop();

    int     getFrameWidth();
    int     getFrameHeight();
    int     getLineSize(int planeNr);

    double  getVideoClock();
    float   getProgress();
    float   getDuration();

	void bufferPackets();
	void printQueueSize();
private:
    void readPackets();
    bool queuePacket(std::queue<AVPacket>& packetQueue, AVPacket* packet);
    bool queueVideoPacket(AVPacket* packet);
    bool popPacket(std::queue<AVPacket>& packetQueue, AVPacket* packet);
    bool popVideoPacket(AVPacket* packet);
    void clearQueue(std::queue<AVPacket>& packetQueue);
    void createAVFrame(AVFrame** avFrame, int width, int height, PixelFormat format);

    bool initializeVideo();

    bool decodeVideoPacket(AVPacket& packet);
    void convertVideoFrame(PixelFormat target);

private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
    AVCodec*                m_pVideoCodec;
    AVStream*               m_pVideoStream;
    AVFrame*                m_pFrame;

    int                     m_MaxVideoQueueSize;
    std::queue<AVPacket>    m_VideoQueue;
    std::mutex              m_VideoQueueMutex;
    std::mutex              m_DecodeVideoMutex;
    std::thread*            m_pPacketReaderThread;

    bool                    m_Stop;
	bool					m_Loop;

    double                  m_VideoClock;
};


