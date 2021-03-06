#pragma once

#include <memory>

#include <glm/glm.hpp>

#include <map>
#include <string>

class SequenceRenderer;
class StimulusRenderer;
class Pass;
class Shader;
class TextureManager;
class ShaderManager;
class Texture2D;

class PassRenderer {
    std::shared_ptr<StimulusRenderer> stimulusRenderer;

    unsigned int                iFrame;
    std::shared_ptr<Pass const> pass;

    Shader* stimulusGeneratorShader;
    Shader* timelineShader;

    using TextureSettings = std::map<std::string, Texture2D*>;
    TextureSettings textureSettings;

    PassRenderer (std::shared_ptr<StimulusRenderer> stimulusRenderer, std::shared_ptr<Pass const> pass,
                  std::shared_ptr<ShaderManager> shaderManager, std::shared_ptr<TextureManager> textureManager);

    Texture2D* polytex;

    //MovieDecoder movieDecoder;
    Texture2D* videoFrameY;
    Texture2D* videoFrameU;
    Texture2D* videoFrameV;
    //VideoFrame   videoFrame;
    glm::vec2 videoClipFactorY;
    glm::vec2 videoClipFactorUV;
    void      _renderPass (float time, uint32_t& slot);

public:
    GEARS_SHARED_CREATE (PassRenderer);
    ~PassRenderer ();

    void renderPass (int skippedFrames, int offset = 0);
    void renderSample (uint32_t sFrame);
    void renderTimeline (uint32_t startFrame, uint32_t frameCount);

    unsigned int getCurrentFrame () { return iFrame; }

    std::shared_ptr<Pass const> getPass () const { return pass; }

    void reset ();
    void skipFrames (uint32_t nFramesToSkip);
};
