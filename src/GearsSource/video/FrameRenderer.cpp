#include "FrameRenderer.h"

#include <assert.h>
#include <math.h>
#include <iostream>
#include <GL/glu.h>

#include "Common/fileoperations.h"
#include "Common/numericoperations.h"
#include "MovieDecoder.h"
#include "videoFrame.h"

const float ROTATE_SPEED = 2.f;

using namespace std;
using namespace GL;

GLuint FrameRenderer::m_DrawBuffer[2];

FrameRenderer::FrameRenderer(int width, int height)
: m_FrameWidth(0)
, m_FrameHeight(0)
, m_WindowWidth(width)
, m_WindowHeight(height)
, m_ScaledWidth(width)
, m_ScaledHeight(height)
, m_StretchToFit(false)
, m_Brightness(0.f)
, m_Gamma(1.f)
, m_Contrast(1.f)
, m_FlipHorizontal(false)
, m_FlipVertical(false)
, m_XRotation(0.f)
, m_YRotation(0.f)
, m_RightTexcoordYPlane(1.f)
, m_RightTexcoordUVPlane(1.f)
, m_RenderSizeChanged(false)
, m_RenderShader(DATADIR"/glover/shaders/framerender.frag", true)
, m_ReadTex(0)
, m_WriteTex(1)
{
    m_DrawBuffer[0] = GL_COLOR_ATTACHMENT0_EXT;
    m_DrawBuffer[1] = GL_COLOR_ATTACHMENT1_EXT;

    m_YPlane.setTextureUnit(GL_TEXTURE0);
    m_UPlane.setTextureUnit(GL_TEXTURE1);
    m_VPlane.setTextureUnit(GL_TEXTURE2);

    m_YPlane.setFormat(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE);
    m_UPlane.setFormat(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE);
    m_VPlane.setFormat(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE);

    m_RenderTargets[0].setFormat(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    m_RenderTargets[1].setFormat(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

    initShaders();
}

FrameRenderer::~FrameRenderer()
{
    destroyShaders();
}

void FrameRenderer::resizeMovieTextures(const VideoFrame& frame)
{
    assert(frame.getYLineSize() && frame.getULineSize() && frame.getVLineSize());
    assert(frame.getULineSize() == frame.getVLineSize());

    m_YPlane.resize(frame.getYLineSize(), m_FrameHeight);
    m_UPlane.resize(frame.getULineSize(), m_FrameHeight / 2);
    m_VPlane.resize(frame.getVLineSize(), m_FrameHeight / 2);

    // adjust right hand texture coordinates to eliminate rendering stride data
    m_RightTexcoordYPlane = (GLfloat) m_FrameWidth / frame.getYLineSize();
    m_RightTexcoordUVPlane = (GLfloat) m_FrameWidth / 2.f / frame.getULineSize();

    map<string, FragmentShader*>::iterator iter;
    for (iter = m_FragmentShaders.begin(); iter != m_FragmentShaders.end(); ++iter)
    {
        iter->second->setUniformVariable("pixelOffsetS", 1.f / frame.getYLineSize());
        iter->second->setUniformVariable("pixelOffsetT", 1.f / m_FrameHeight);
    }
}

void FrameRenderer::initRenderTextures()
{
    m_Fbo.bind();

    m_RenderTargets[0].resize(m_ScaledWidth, m_ScaledHeight);
    m_RenderTargets[1].resize(m_ScaledWidth, m_ScaledHeight);

    m_Fbo.attachTexture(m_RenderTargets[0].getId(), GL_TEXTURE_2D, m_DrawBuffer[0]);
    m_Fbo.attachTexture(m_RenderTargets[1].getId(), GL_TEXTURE_2D, m_DrawBuffer[1]);

    assert(FramebufferObject::statusOK());
    m_Fbo.unbind();
}

void FrameRenderer::updateFrame(const VideoFrame& frame)
{
    bool frameSizeChanged = frame.getWidth() != m_FrameWidth;
    frameSizeChanged = frameSizeChanged || frame.getHeight() != m_FrameHeight;

    m_FrameWidth = frame.getWidth();
    m_FrameHeight = frame.getHeight();

    if (frameSizeChanged)
    {
        calculateDimensions();
        resizeMovieTextures(frame);
    }

    uploadTextureData(frame);
    processFrame();
}

void FrameRenderer::initShaders()
{
    m_RenderShader.compile();

    m_RenderShader.addUniformVariable("texUnit1");
    m_RenderShader.addUniformVariable("texUnit2");
    m_RenderShader.addUniformVariable("texUnit3");
    m_RenderShader.addUniformVariable("brightness");
    m_RenderShader.addUniformVariable("gamma");
    m_RenderShader.addUniformVariable("contrast");

    m_RenderShader.setUniformVariable("texUnit1", PLANE1);
    m_RenderShader.setUniformVariable("texUnit2", PLANE2);
    m_RenderShader.setUniformVariable("texUnit3", PLANE3);
    m_RenderShader.setUniformVariable("brightness", m_Brightness);
    m_RenderShader.setUniformVariable("gamma", m_Gamma);
    m_RenderShader.setUniformVariable("contrast", m_Contrast);

    assert(glGetError() == 0);
}

void FrameRenderer::toggleShader(const string& filename)
{
    if (m_FragmentShaders.find(filename) == m_FragmentShaders.end())
    {
        addShader(filename);
    }
    else
    {
        removeShader(filename);
    }
}

void FrameRenderer::addShader(const string& filename)
{
    if (m_FragmentShaders.find(filename) == m_FragmentShaders.end())
    {
        FragmentShader* pShader = new FragmentShader(filename, true);
        if (pShader->compile())
        {
            pShader->addUniformVariable("texUnit1");
            pShader->addUniformVariable("pixelOffsetS");
            pShader->addUniformVariable("pixelOffsetT");
            pShader->setUniformVariable("texUnit1", 0);
            pShader->setUniformVariable("pixelOffsetS", 1.f / m_FrameWidth);
            pShader->setUniformVariable("pixelOffsetS", 1.f / m_FrameHeight);
            m_FragmentShaders[filename] = pShader;
        }
        else
        {
            delete pShader;
        }
    }
}

void FrameRenderer::removeShader(const string& filename)
{
    map<string, FragmentShader*>::iterator iter = m_FragmentShaders.find(filename);
    if (iter != m_FragmentShaders.end())
    {
        m_FragmentShaders.erase(iter);
    }
}

void FrameRenderer::destroyShaders()
{
    map<string, FragmentShader*>::iterator iter = m_FragmentShaders.end();
    while (iter != m_FragmentShaders.end())
    {
        delete iter->second;
        m_FragmentShaders.erase(iter);
    }
}

void FrameRenderer::setWindowSize(int width, int height)
{
    if (width == m_WindowWidth && height == m_WindowHeight)
    {
        return;
    }

    m_WindowWidth = width;
    m_WindowHeight = height;
    calculateDimensions();
    m_RenderSizeChanged = true;
}

void FrameRenderer::setStretchToFit(bool enabled)
{
    m_StretchToFit = enabled;
    calculateDimensions();
    m_RenderSizeChanged = true;
}

void FrameRenderer::flipHorizontal()
{
    m_FlipHorizontal = !m_FlipHorizontal;
}

void FrameRenderer::flipVertical()
{
    m_FlipVertical = !m_FlipVertical;
}

void FrameRenderer::adjustBrightness(float offset)
{
    m_Brightness += offset;
    NumericOperations::clip(m_Brightness, -1.f, 1.f);
    m_RenderShader.setUniformVariable("brightness", m_Brightness);
}

void FrameRenderer::adjustGamma(float offset)
{
    m_Gamma += offset;
    NumericOperations::clip(m_Gamma, 0.f, 10.f);
    m_RenderShader.setUniformVariable("gamma", m_Gamma);
}

void FrameRenderer::adjustContrast(float offset)
{
    m_Contrast += offset;
    NumericOperations::clip(m_Contrast, 0.f, 10.f);
    m_RenderShader.setUniformVariable("contrast", m_Contrast);
}

void FrameRenderer::resetImageSettings()
{
    m_Brightness = 0.0f;
    m_Gamma = 1.0f;
    m_Contrast = 1.0f;
    m_RenderShader.setUniformVariable("brightness", m_Brightness);
    m_RenderShader.setUniformVariable("gamma", m_Gamma);
    m_RenderShader.setUniformVariable("contrast", m_Contrast);
}

void FrameRenderer::calculateDimensions()
{
    if (m_StretchToFit)
    {
        m_XOffset = 0.f;
        m_YOffset = 0.f;
        m_ScaledWidth = m_WindowWidth;
        m_ScaledHeight = m_WindowHeight;
        initRenderTextures();

        return;
    }
    assert(m_WindowHeight);

    if (m_FrameHeight != 0)
    {
        float windowAspect = (float) m_WindowWidth / (float) m_WindowHeight;
        float movieAspect = (float) m_FrameWidth / (float) m_FrameHeight;
        float multiplier;

        if (movieAspect < windowAspect)
        {
            multiplier = (float) m_WindowHeight / (float) m_FrameHeight;
        }
        else
        {
            multiplier = (float) m_WindowWidth / (float) m_FrameWidth;
        }

        m_ScaledWidth = (int) (m_FrameWidth * multiplier);
        m_ScaledHeight = (int) (m_FrameHeight * multiplier);

        m_XOffset = (m_WindowWidth - m_ScaledWidth) / 2.f;
        m_YOffset = (m_WindowHeight - m_ScaledHeight) / 2.f;

        initRenderTextures();
    }
}

void FrameRenderer::updateRotations(uint64 elapsedTime)
{
    float adjust = elapsedTime * 0.18f * ROTATE_SPEED;

    m_XRotation += m_FlipVertical ? adjust : -adjust;
    m_YRotation += m_FlipHorizontal ? adjust : -adjust;

    NumericOperations::clip(m_XRotation, 0.f, 180.f);
    NumericOperations::clip(m_YRotation, 0.f, 180.f);
}

void FrameRenderer::applyRotations(uint64 elapsedTime)
{
    float xCenter = m_WindowWidth / 2.f;
    float yCenter = m_WindowHeight / 2.f;
    glTranslatef(xCenter, yCenter, 0.f);

    updateRotations(elapsedTime);
    if (m_YRotation > 0)
    {
        float percentage = (sin((m_YRotation / 180.f * 3.14159265f) + 3.14159265f * 1.5f) + 1) * 90.f;
        glRotatef(percentage, 0.f, 1.f, 0.f);
    }

    if (m_XRotation > 0)
    {
        float percentage = (sin((m_XRotation / 180.f * 3.14159265f) + 3.14159265f * 1.5f) + 1) * 90.f;
        glRotatef(percentage, 1.f, 0.f, 0.f);
    }

    glTranslatef(-xCenter, -yCenter, 0.f);
}

void FrameRenderer::renderFrame(uint64 elapsedTime)
{
    if (m_FrameWidth == 0 || m_FrameHeight == 0)
    {
        return;
    }

    if (m_RenderSizeChanged)
    {
        processFrame();
        m_RenderSizeChanged = false;
    }

    applyRotations(elapsedTime);
    glTranslatef(m_XOffset, m_YOffset, 0.f);

    m_RenderTargets[m_ReadTex].bind();
    drawQuad(m_ScaledWidth - 1.f, m_ScaledHeight - 1.f, 1.f, 1.f);

    Texture2D::unbind();
}

void FrameRenderer::processFrame()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, m_WindowWidth - 1.f, 0, m_WindowHeight - 1.f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT);

    m_Fbo.bind();
    glDrawBuffer(m_DrawBuffer[m_WriteTex]);
    glUseProgram(m_RenderShader.getProgramId());

    m_YPlane.bind();
    m_UPlane.bind();
    m_VPlane.bind();

    drawQuad((float) m_ScaledWidth, (float) m_ScaledHeight, (float) m_RightTexcoordYPlane, 1.f, (float) m_RightTexcoordUVPlane, 1.f);
    swapRenderTarget();

    map<string, FragmentShader*>::iterator iter;
    for (iter = m_FragmentShaders.begin(); iter != m_FragmentShaders.end(); ++iter)
    {
        glDrawBuffer(m_DrawBuffer[m_WriteTex]);
        glUseProgram((*iter).second->getProgramId());
        m_RenderTargets[m_ReadTex].bind();

        drawQuad(m_ScaledWidth - 1, m_ScaledHeight - 1, 1.f, 1.f);
        swapRenderTarget();
    }

    Texture2D::unbind();
    glUseProgram(GL_NONE);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();

    m_Fbo.unbind();
    assert(glGetError() == 0);
}

void FrameRenderer::uploadTextureData(const VideoFrame& frame)
{
    m_YPlane.setTextureData(frame.getYPlane());
    m_UPlane.setTextureData(frame.getUPlane());
    m_VPlane.setTextureData(frame.getVPlane());
}

void FrameRenderer::drawQuad(float width, float height, float texWidth, float texHeight)
{
    glBegin(GL_QUADS);
    {
        glTexCoord2f(     0.f,       0.f); glVertex2f(  0.f,    0.f);
        glTexCoord2f(texWidth,       0.f); glVertex2f(width,    0.f);
        glTexCoord2f(texWidth, texHeight); glVertex2f(width, height);
        glTexCoord2f(     0.f, texHeight); glVertex2f(  0.f, height);
    }
    glEnd();
}

void FrameRenderer::drawQuad(float width, float height, float texWidth1, float texHeight1, float texWidth2, float texHeight2)
{
    glBegin(GL_QUADS);
    {
        glMultiTexCoord2f(GL_TEXTURE0, 0.f, texHeight1);
        glMultiTexCoord2f(GL_TEXTURE1, 0.f, texHeight2);
        glVertex2f(0.f, 0.f);
        glMultiTexCoord2f(GL_TEXTURE0, texWidth1, texHeight1);
        glMultiTexCoord2f(GL_TEXTURE1, texWidth2, texHeight2);
        glVertex2f(width, 0.f);
        glMultiTexCoord2f(GL_TEXTURE0, texWidth1, 0.f);
        glMultiTexCoord2f(GL_TEXTURE1, texWidth2, 0.f);
        glVertex2f(width, height);
        glMultiTexCoord2f(GL_TEXTURE0, 0.f, 0.f);
        glMultiTexCoord2f(GL_TEXTURE1, 0.f, 0.f);
        glVertex2f(0.f, height);
    }
    glEnd();
}

void FrameRenderer::swapRenderTarget()
{
    short temp = m_ReadTex;
    m_ReadTex = m_WriteTex;
    m_WriteTex = temp;
}
