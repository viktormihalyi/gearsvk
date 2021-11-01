#ifndef SEQUENCERUNNER_MOVIEEXPORTER_HPP
#define SEQUENCERUNNER_MOVIEEXPORTER_HPP

#include <vector>
#include <filesystem>
#include <memory>

class MovieExporter {
private:
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    struct VideoSettings {
        size_t width;
        size_t height;
        size_t fps;
        size_t bitrate;
    };

public:
    MovieExporter (const std::filesystem::path& exportFilePath, VideoSettings videoSettings);

    ~MovieExporter ();

    void PushFrame (std::vector<uint8_t> frame);

};

#endif