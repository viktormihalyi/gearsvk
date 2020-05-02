#ifndef FRAMERENDERER_H
#define FRAMERENDERER_H

#include <GLee.h>
#include <string>
#include <map>

#include "GLLib/fragmentshader.h"
#include "GLLib/texture2d.h"
#include "GLLib/framebufferobject.h"

class VideoFrame;

class FrameRenderer
{
    enum
    {
        PLANE1,
        PLANE2,
        PLANE3,
        NRPLANES
    };

public:
    FrameRenderer(int width, int height);
    ~FrameRenderer();

    void updateFrame(const VideoFrame& frame);
    void renderFrame(uint64 elapsedTime);
    void setWindowSize(int width, int height);
    void setStretchToFit(bool enabled);
    void flipHorizontal();
    void flipVertical();
    void adjustBrightness(float offset);
    void adjustGamma(float offset);
    void adjustContrast(float offset);
    void resetImageSettings();
    void toggleShader(const std::string& filename);

private:
    void resizeMovieTextures(const VideoFrame& frame);
    void initRenderTextures();
    void initShaders();
    void destroyShaders();
    void calculateDimensions();
    void uploadTextureData(const VideoFrame& frame);
    void updateRotations(uint64 elapsedTime);
    void applyRotations(uint64 elapsedTime);
    void processFrame();
    void drawQuad(float width, float height, float texWidth, float texHeight);
    void drawQuad(float width, float height, float texWidth1, float texHeight1, float texWidth2, float texHeight2);
    void swapRenderTarget();
    void addShader(const std::string& filename);
    void removeShader(const std::string& filename);

    int                 m_FrameWidth;
    int                 m_FrameHeight;
    int                 m_WindowWidth;
    int                 m_WindowHeight;
    int                 m_ScaledWidth;
    int                 m_ScaledHeight;
    int                 m_LineSize[NRPLANES];
    float               m_XOffset;
    float               m_YOffset;
    bool                m_StretchToFit;

    float               m_Brightness;
    float               m_Gamma;
    float               m_Contrast;

    bool                m_FlipHorizontal;
    bool                m_FlipVertical;
    float               m_XRotation;
    float               m_YRotation;

    static GLuint       m_DrawBuffer[2];

    GL::Texture2D       m_RenderTargets[2];
    GL::Texture2D       m_YPlane;
    GL::Texture2D       m_UPlane;
    GL::Texture2D       m_VPlane;

    GLfloat             m_RightTexcoordYPlane;
    GLfloat             m_RightTexcoordUVPlane;

    bool                m_RenderSizeChanged;

    GL::FragmentShader      m_RenderShader;
    GL::FramebufferObject   m_Fbo;

    short               m_ReadTex;
    short               m_WriteTex;

    std::map<std::string, GL::FragmentShader*> m_FragmentShaders;
};

#endif
