#include "MovieExporter.hpp"

#include "Utils/Assert.hpp"

#include <cstdint>
#include <stdexcept>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <x264.h>
}


struct MovieExporter::Impl {
    std::filesystem::path exportFilePath;
    VideoSettings videoSettings;
    size_t frameCounter;

    AVOutputFormat* outputFormat;
    AVFormatContext* formatContext;
    AVCodec* codec;
    AVStream* stream;
    AVCodecContext* codecContext;
    AVFrame* videoFrame;
    SwsContext* swsContext;
};


MovieExporter::MovieExporter (const std::filesystem::path& exportFilePath, VideoSettings videoSettings)
    : impl { std::make_unique<Impl> () }
{
    impl->exportFilePath = exportFilePath;
    impl->videoSettings = videoSettings;
    impl->frameCounter = 0;

    const std::string filename = exportFilePath.filename ().string ();

    impl->outputFormat = av_guess_format (nullptr, filename.c_str (), nullptr);
    if GVK_ERROR (impl->outputFormat == nullptr)
        throw std::runtime_error ("");

    impl->formatContext = nullptr;
    if GVK_ERROR (avformat_alloc_output_context2 (&impl->formatContext, impl->outputFormat, nullptr, filename.c_str ()) < 0 || impl->formatContext == nullptr)
        throw std::runtime_error ("");

    impl->codec = avcodec_find_encoder (impl->outputFormat->video_codec);
    if GVK_ERROR (impl->codec == nullptr)
        throw std::runtime_error ("");

    impl->stream = avformat_new_stream (impl->formatContext, impl->codec);
    if GVK_ERROR (impl->stream == nullptr)
        throw std::runtime_error ("");

    impl->codecContext = avcodec_alloc_context3 (impl->codec);
    if GVK_ERROR (impl->codecContext == nullptr)
        throw std::runtime_error ("");

    impl->stream->codecpar->codec_id   = impl->outputFormat->video_codec;
    impl->stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    impl->stream->codecpar->width      = impl->videoSettings.width;
    impl->stream->codecpar->height     = impl->videoSettings.height;
    impl->stream->codecpar->format     = AV_PIX_FMT_YUV420P;
    impl->stream->codecpar->bit_rate   = impl->videoSettings.bitrate * 1000;

    if GVK_ERROR (avcodec_parameters_to_context (impl->codecContext, impl->stream->codecpar) < 0)
        throw std::runtime_error ("");

    impl->codecContext->time_base    = AVRational { 1, 1 };
    impl->codecContext->max_b_frames = 2;
    impl->codecContext->gop_size     = 12;
    impl->codecContext->framerate    = AVRational { static_cast<int> (impl->videoSettings.fps), 1 };

    //must remove the following
    //codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (impl->stream->codecpar->codec_id == AV_CODEC_ID_H264) {
        GVK_VERIFY (av_opt_set (impl->codecContext->priv_data, "preset", "ultrafast", 0) == 0);
    } else if (impl->stream->codecpar->codec_id == AV_CODEC_ID_H265) {
        GVK_VERIFY (av_opt_set (impl->codecContext->priv_data, "preset", "ultrafast", 0) == 0);
    }

    if GVK_ERROR (avcodec_parameters_from_context (impl->stream->codecpar, impl->codecContext) < 0)
        throw std::runtime_error ("");

    if GVK_ERROR (avcodec_open2 (impl->codecContext, impl->codec, NULL) < 0)
        throw std::runtime_error ("");

    if (!(impl->outputFormat->flags & AVFMT_NOFILE))
        if GVK_ERROR (avio_open (&impl->formatContext->pb, filename.c_str (), AVIO_FLAG_WRITE) < 0)
            throw std::runtime_error ("");

    if GVK_ERROR (avformat_write_header (impl->formatContext, NULL) < 0)
        throw std::runtime_error ("");

    av_dump_format (impl->formatContext, 0, filename.c_str (), 1);
        
    impl->videoFrame = av_frame_alloc ();
    if GVK_ERROR (impl->videoFrame == nullptr)
        throw std::runtime_error ("");
        
    impl->videoFrame->format = AV_PIX_FMT_YUV420P;
    impl->videoFrame->width  = impl->codecContext->width;
    impl->videoFrame->height = impl->codecContext->height;
    if GVK_ERROR (av_frame_get_buffer (impl->videoFrame, 32) < 0)
        throw std::runtime_error ("");

    impl->swsContext = sws_getContext (impl->codecContext->width, impl->codecContext->height, AV_PIX_FMT_RGB24,
                                       impl->codecContext->width, impl->codecContext->height, AV_PIX_FMT_YUV420P,
                                       SWS_BICUBIC, 0, 0, 0);
        
    if GVK_ERROR (impl->swsContext == nullptr)
        throw std::runtime_error ("");
}


MovieExporter::~MovieExporter ()
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


void MovieExporter::PushFrame (std::vector<uint8_t> frame)
{
    uint8_t* data = frame.data ();
    int inLinesize = 3 * impl->codecContext->width;
    sws_scale (impl->swsContext,
               (const uint8_t* const*)&data,
               &inLinesize,
               0,
               impl->codecContext->height,
               impl->videoFrame->data,
               impl->videoFrame->linesize);

    impl->videoFrame->pts = (1.0 / impl->videoSettings.fps) * 90000 * (impl->frameCounter++);

    std::cout
        << impl->videoFrame->pts << " "
        << impl->codecContext->time_base.num << " "
        << impl->codecContext->time_base.den << " "
        << impl->frameCounter << std::endl;

    if GVK_ERROR (avcodec_send_frame (impl->codecContext, impl->videoFrame) != 0)
        return;

    AVPacket pkt;
    av_init_packet (&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    pkt.flags |= AV_PKT_FLAG_KEY;

    if (avcodec_receive_packet (impl->codecContext, &pkt) == 0) {
        static int counter = 0;
        if (counter == 0) {
            FILE* fp = fopen ("dump_first_frame1.dat", "wb");
            if GVK_ERROR (fp == nullptr)
                return;
            fwrite (pkt.data, pkt.size, 1, fp);
            GVK_VERIFY (fclose (fp) == 0);
        }
        std::cout << "pkt key: " << (pkt.flags & AV_PKT_FLAG_KEY) << " " << pkt.size << " " << (counter++) << std::endl;
        uint8_t* size = ((uint8_t*)pkt.data);
        std::cout << "first: " << (int)size[0] << " " << (int)size[1] << " " << (int)size[2] << " " << (int)size[3] << " " << (int)size[4] << " " << (int)size[5] << " " << (int)size[6] << " " << (int)size[7] << std::endl;
        GVK_VERIFY (av_interleaved_write_frame (impl->formatContext, &pkt) == 0);
        av_packet_unref (&pkt);
    }
}
