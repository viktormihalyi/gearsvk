#pragma once

#include "Ptr.hpp"
#include "stdafx.h"

#include "glmlib.hpp"

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
    Ptr<StimulusRenderer> stimulusRenderer;

    unsigned int iFrame;
    PtrC<Pass>   pass;

    Shader* stimulusGeneratorShader;
    Shader* timelineShader;

    using TextureSettings = std::map<std::string, Texture2D*>;
    TextureSettings textureSettings;

    PassRenderer (Ptr<StimulusRenderer> stimulusRenderer, PtrC<Pass> pass,
                  Ptr<ShaderManager> shaderManager, Ptr<TextureManager> textureManager);

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

    PtrC<Pass> getPass () const { return pass; }

    void reset ();
    void skipFrames (uint nFramesToSkip);
};
