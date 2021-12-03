#ifndef SEQUENCERUNNER_VIDEOEXPORTER_HPP
#define SEQUENCERUNNER_VIDEOEXPORTER_HPP

#include <vector>
#include <filesystem>
#include <memory>

class VideoExporter {
private:
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    struct VideoSettings {
        uint32_t width;
        uint32_t height;
        uint32_t fps;
        uint32_t bitrate;
    };

public:
    VideoExporter (const std::filesystem::path& exportFilePath, VideoSettings videoSettings);

    ~VideoExporter ();

    void PushFrame (const std::vector<uint8_t>& frame);

};

#endif