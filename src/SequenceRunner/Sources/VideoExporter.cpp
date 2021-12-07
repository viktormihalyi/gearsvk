#include "VideoExporter.hpp"

#include "Utils/Assert.hpp"

#include <cstdint>
#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <x264.h>
}

#include "spdlog/spdlog.h"

struct VideoExporter::Impl {
    std::filesystem::path exportFilePath;
    VideoSettings         videoSettings;
    size_t                frameCounter;

    AVOutputFormat*  outputFormat;          // libavformat
    AVFormatContext* formatContext;         // libavformat
    AVCodec*         codec;                 // libavcodec
    AVStream*        stream;                // libavformat
    AVCodecContext*  codecContext;          // libavcodec
    AVFrame*         videoFrame;            // libavutil
    SwsContext*      swsContext;            // libswscale
};


VideoExporter::VideoExporter (const std::filesystem::path& exportFilePath, VideoSettings videoSettings)
    : impl { std::make_unique<Impl> () }
{
    impl->exportFilePath = exportFilePath;
    impl->videoSettings  = videoSettings;
    impl->frameCounter   = 0;

    const std::string filename = exportFilePath.filename ().string ();

    av_log_set_level (AV_LOG_WARNING);

    impl->outputFormat = av_guess_format (nullptr, filename.c_str (), nullptr);
    if GVK_ERROR (impl->outputFormat == nullptr)
        throw std::runtime_error ("av_guess_format failed");

    impl->formatContext = nullptr;
    if GVK_ERROR (avformat_alloc_output_context2 (&impl->formatContext, impl->outputFormat, nullptr, filename.c_str ()) < 0 || impl->formatContext == nullptr)
        throw std::runtime_error ("avformat_alloc_output_context2 failed");

    impl->codec = avcodec_find_encoder (impl->outputFormat->video_codec);
    if GVK_ERROR (impl->codec == nullptr)
        throw std::runtime_error ("avcodec_find_encoder failed");

    impl->stream = avformat_new_stream (impl->formatContext, impl->codec);
    if GVK_ERROR (impl->stream == nullptr)
        throw std::runtime_error ("avformat_new_stream failed");

    impl->codecContext = avcodec_alloc_context3 (impl->codec);
    if GVK_ERROR (impl->codecContext == nullptr)
        throw std::runtime_error ("avcodec_alloc_context3 failed");

    impl->stream->codecpar->codec_id   = impl->outputFormat->video_codec;
    impl->stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    impl->stream->codecpar->width      = impl->videoSettings.width;
    impl->stream->codecpar->height     = impl->videoSettings.height;
    impl->stream->codecpar->format     = AV_PIX_FMT_YUV420P;
    impl->stream->codecpar->bit_rate   = impl->videoSettings.bitrate;

    if GVK_ERROR (avcodec_parameters_to_context (impl->codecContext, impl->stream->codecpar) < 0)
        throw std::runtime_error ("avcodec_parameters_to_context failed");

    impl->codecContext->time_base    = AVRational { 1, 1 };
    impl->codecContext->max_b_frames = 2;
    impl->codecContext->gop_size     = 12;
    impl->codecContext->framerate    = AVRational { static_cast<int> (impl->videoSettings.fps), 1 };

    // must remove the following
    // impl->codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    std::string crf = "23";
    
    // read from envrionment variable
    if (const char* envVar = std::getenv ("GVK_CRF")) {
        crf = std::string (envVar);
    }

    if (impl->stream->codecpar->codec_id == AV_CODEC_ID_H264) {
        GVK_VERIFY (av_opt_set (impl->codecContext->priv_data, "preset", "ultrafast", 0) == 0);
        GVK_VERIFY (av_opt_set (impl->codecContext->priv_data, "crf", crf.c_str (), AV_OPT_SEARCH_CHILDREN) == 0);
    } else if (impl->stream->codecpar->codec_id == AV_CODEC_ID_H265) {
        GVK_VERIFY (av_opt_set (impl->codecContext->priv_data, "preset", "ultrafast", 0) == 0);
        GVK_VERIFY (av_opt_set (impl->codecContext->priv_data, "crf", crf.c_str (), AV_OPT_SEARCH_CHILDREN) == 0);
    }

    if GVK_ERROR (avcodec_parameters_from_context (impl->stream->codecpar, impl->codecContext) < 0)
        throw std::runtime_error ("avcodec_parameters_from_context failed");

    if GVK_ERROR (avcodec_open2 (impl->codecContext, impl->codec, NULL) < 0)
        throw std::runtime_error ("avcodec_open2 failed");

    if (!(impl->outputFormat->flags & AVFMT_NOFILE))
        if GVK_ERROR (avio_open (&impl->formatContext->pb, filename.c_str (), AVIO_FLAG_WRITE) < 0)
            throw std::runtime_error ("avio_open failed");

    if GVK_ERROR (avformat_write_header (impl->formatContext, NULL) < 0)
        throw std::runtime_error ("avformat_write_header failed");

    const int index = 0;
    const int isOutput = 1;
    av_dump_format (impl->formatContext, index, filename.c_str (), isOutput);

    impl->videoFrame = av_frame_alloc ();
    if GVK_ERROR (impl->videoFrame == nullptr)
        throw std::runtime_error ("av_frame_alloc failed");

    impl->videoFrame->format = AV_PIX_FMT_YUV420P;
    impl->videoFrame->width  = impl->codecContext->width;
    impl->videoFrame->height = impl->codecContext->height;
    if GVK_ERROR (av_frame_get_buffer (impl->videoFrame, 32) < 0)
        throw std::runtime_error ("av_frame_get_buffer failed");

    impl->swsContext = sws_getContext (impl->codecContext->width,  /* srcW */
                                       impl->codecContext->height, /* srcH */
                                       AV_PIX_FMT_BGRA,            /* srcFormat */
                                       impl->codecContext->width,  /* dstW */
                                       impl->codecContext->height, /* dstH */
                                       AV_PIX_FMT_YUV420P,         /* dstFormat */
                                       SWS_BICUBIC,                /* flags */
                                       nullptr,                    /* srcFilter */
                                       nullptr,                    /* dstFilter */
                                       nullptr);                   /* param */

    if GVK_ERROR (impl->swsContext == nullptr)
        throw std::runtime_error ("sws_getContext");
}


VideoExporter::~VideoExporter ()
{
    AVPacket pkt;
    av_init_packet (&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (true) {
        int err = avcodec_send_frame (impl->codecContext, NULL);
        GVK_VERIFY (err == 0 || err == AVERROR_EOF);
        if (avcodec_receive_packet (impl->codecContext, &pkt) == 0) {
            GVK_VERIFY (av_interleaved_write_frame (impl->formatContext, &pkt) == 0);
            av_packet_unref (&pkt);
        } else {
            break;
        }
    }

    GVK_VERIFY (av_write_trailer (impl->formatContext) == 0);

    if (!(impl->outputFormat->flags & AVFMT_NOFILE))
        GVK_VERIFY (avio_close (impl->formatContext->pb) == 0);

    sws_freeContext (impl->swsContext);
    av_frame_free (&impl->videoFrame);
    avcodec_free_context (&impl->codecContext);
    avformat_free_context (impl->formatContext);
}


void VideoExporter::PushFrame (const std::vector<uint8_t>& frame)
{
    const uint8_t* inData = frame.data ();
    
    const int inLineSize = 4 * impl->codecContext->width;

    sws_scale (impl->swsContext,
               &inData,                     /* srcSlice */
               &inLineSize,                 /* srcStride */
               0,                           /* srcSliceY */
               impl->codecContext->height,  /* srcSliceH */
               impl->videoFrame->data,      /* dst */
               impl->videoFrame->linesize); /* dstStride */

    const size_t mpegClockResolution = 90000;
    impl->videoFrame->pts  = (1.0 / static_cast<double> (impl->videoSettings.fps)) * mpegClockResolution * (impl->frameCounter++);

    if GVK_ERROR (avcodec_send_frame (impl->codecContext, impl->videoFrame) != 0)
        return;

    AVPacket pkt;
    av_init_packet (&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    pkt.flags |= AV_PKT_FLAG_KEY;

    if (avcodec_receive_packet (impl->codecContext, &pkt) == 0) {
#if DUMP_FIRST_FRAME
        static int counter = 0;
        if (counter == 0) {
            FILE* fp = fopen ("dump_first_frame1.dat", "wb");
            if GVK_ERROR (fp == nullptr)
                return;
            fwrite (pkt.data, pkt.size, 1, fp);
            GVK_VERIFY (fclose (fp) == 0);
        }
        counter++;
#endif

        GVK_VERIFY (av_interleaved_write_frame (impl->formatContext, &pkt) == 0);
        av_packet_unref (&pkt);
    }
}
