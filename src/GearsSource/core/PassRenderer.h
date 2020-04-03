#pragma once

//#include "TextureQueue.hpp"
//#include "StimulusGrid.hpp"
//#include "Shader.hpp"
//#include "Texture.hpp"
//#include "Framebuffer.hpp"
//#include "Pointgrid.hpp"
//#include "Quad.hpp"

//#include "MovieDecoder.h"

#include "stdafx.h"

#include "Pass.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include <map>
#include <string>

class SequenceRenderer;
class StimulusRenderer;

class PassRenderer {
    std::shared_ptr<StimulusRenderer> stimulusRenderer;

    unsigned int iFrame;
    Pass::CP     pass;

    Shader* stimulusGeneratorShader;
    Shader* timelineShader;

    using TextureSettings = std::map<std::string, Texture2D*>;
    TextureSettings textureSettings;

    PassRenderer (std::shared_ptr<StimulusRenderer> stimulusRenderer, Pass::CP pass,
                  ShaderManager::P shaderManager, TextureManager::P textureManager);

    Texture2D* polytex;

    //MovieDecoder movieDecoder;
    Texture2D* videoFrameY;
    Texture2D* videoFrameU;
    Texture2D* videoFrameV;
    //VideoFrame   videoFrame;
    glm::vec2 videoClipFactorY;
    glm::vec2 videoClipFactorUV;
    void      _renderPass (float time, uint& slot);

public:
    GEARS_SHARED_CREATE (PassRenderer);
    ~PassRenderer ();

    void renderPass (int skippedFrames, int offset = 0);
    void renderSample (uint sFrame);
    void renderTimeline (uint startFrame, uint frameCount);

    unsigned int getCurrentFrame () { return iFrame; }

    Pass::CP getPass () const { return pass; }

    void reset ();
    void skipFrames (uint nFramesToSkip);
};
