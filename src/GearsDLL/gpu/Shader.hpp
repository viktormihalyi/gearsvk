
#ifndef _SHADER_
#define _SHADER_

#include <string>

#include "GearsDLL/OpenGLProxy.hpp"

#include "VulkanWrapper.hpp"

class Shader {
private:
    struct Type {
        std::string path;
        std::string source;
    };

    struct {
        Type vertex;
        Type geometry;
        Type fragment;
    } info;

    ShaderPipeline::U ppl;

    GLuint shaderProgram;
    GLuint vertexProgram;
    GLuint fragmentProgram;
    GLuint geometryProgram;
    GLint  geomShaderOutputType;
    Shader ();
    bool fileToString (const char* path, char*& out, int& len);

public:
    Shader (const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath = 0, GLint geomShaderOutputType = GL_LINE_STRIP);
    Shader (std::string vertexShaderPath, std::string fragmentShaderSource);
    Shader (std::string vertexShaderSource, std::string fragmentShaderSource, bool dummy);
    Shader (std::string vertexShaderSource, std::string geometryShaderSource, std::string fragmentShaderSource, GLint geomShaderOutputType = GL_TRIANGLE_STRIP);

    virtual ~Shader ();

    GLuint getProgram () { return shaderProgram; }
    GLuint getGeometryProgram () { return geometryProgram; }

private:
    void shaderFromFile (const char* shaderPath, GLenum shaderType, GLuint& handle);
    void shaderFromString (const char* shaderSource, int len, GLenum shaderType, GLuint& handle);
    void linkShaders (GLuint& vertexShader, GLuint& fragmentShader, GLuint& geometryShader, GLuint& handle, GLint geomShaderOutputType);

    std::string getShaderInfoLog (GLuint& object);
    std::string getProgramInfoLog (GLuint& object);

public:
    void enable ();
    void disable ();

    GLuint getHandle ()
    {
        return shaderProgram;
    }

    void bindUniformBool (const char* name, bool b);
    void bindUniformUint (const char* name, GLuint i);
    void bindUniformUint2 (const char* name, GLuint i, GLuint j);
    void bindUniformInt (const char* name, int i);
    void bindUniformInt2 (const char* name, int i1, int i2);
    void bindUniformFloat (const char* name, float f);
    void bindUniformFloat2 (const char* name, float f1, float f2);
    void bindUniformFloat3 (const char* name, float f1, float f2, float f3);
    void bindUniformTexture (const char* name, GLuint texture, GLuint unit);
    void bindUniformTexture1D (const char* name, GLuint texture, GLuint unit);
    void bindUniformTextureRect (const char* name, GLuint texture, GLuint unit);
    void bindUniformTextureArray (const char* name, GLuint texture, GLuint unit);

    void bindUniformMatrix (const char* name, const float* m, unsigned int arraySize = 1);
    void bindUniformVector (const char* name, const float* m, unsigned int arraySize = 1);
    void bindUniformFloat4Array (const char* name, const float* m, unsigned int arraySize = 1);
    void bindUniformFloatArray (const char* name, const float* m, unsigned int arraySize = 1);
    void bindUniformIntArray (const char* name, const int* iv, unsigned int arraySize);

    void bindAttribLocation (GLuint id, const char* name);
};

#endif
