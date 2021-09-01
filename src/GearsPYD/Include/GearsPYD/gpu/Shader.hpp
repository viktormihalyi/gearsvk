
#ifndef _SHADER_
#define _SHADER_

#include <string>

#include "OpenGLProxy.hpp"

#include "Event.hpp"


class UniformBinder {
public:
    virtual ~UniformBinder () = default;

    virtual void bindUniformBool (const char* name, bool b)                                            = 0;
    virtual void bindUniformUint (const char* name, GLuint i)                                          = 0;
    virtual void bindUniformUint2 (const char* name, GLuint i, GLuint j)                               = 0;
    virtual void bindUniformInt (const char* name, int i)                                              = 0;
    virtual void bindUniformInt2 (const char* name, int i1, int i2)                                    = 0;
    virtual void bindUniformFloat (const char* name, float f)                                          = 0;
    virtual void bindUniformFloat2 (const char* name, float f1, float f2)                              = 0;
    virtual void bindUniformFloat3 (const char* name, float f1, float f2, float f3)                    = 0;
    virtual void bindUniformTexture (const char* name, GLuint texture, GLuint unit)                    = 0;
    virtual void bindUniformTexture1D (const char* name, GLuint texture, GLuint unit)                  = 0;
    virtual void bindUniformTextureRect (const char* name, GLuint texture, GLuint unit)                = 0;
    virtual void bindUniformTextureArray (const char* name, GLuint texture, GLuint unit)               = 0;
    virtual void bindUniformMatrix (const char* name, const float* m, unsigned int arraySize = 1)      = 0;
    virtual void bindUniformVector (const char* name, const float* m, unsigned int arraySize = 1)      = 0;
    virtual void bindUniformFloat4Array (const char* name, const float* m, unsigned int arraySize = 1) = 0;
    virtual void bindUniformFloatArray (const char* name, const float* m, unsigned int arraySize = 1)  = 0;
    virtual void bindUniformIntArray (const char* name, const int* iv, unsigned int arraySize)         = 0;
};

class Shader : public UniformBinder {
public:
    static GVK::Event<std::string> uniformBoundEvent;

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

    void bindUniformBool (const char* name, bool b) override;
    void bindUniformUint (const char* name, GLuint i) override;
    void bindUniformUint2 (const char* name, GLuint i, GLuint j) override;
    void bindUniformInt (const char* name, int i) override;
    void bindUniformInt2 (const char* name, int i1, int i2) override;
    void bindUniformFloat (const char* name, float f) override;
    void bindUniformFloat2 (const char* name, float f1, float f2) override;
    void bindUniformFloat3 (const char* name, float f1, float f2, float f3) override;
    void bindUniformTexture (const char* name, GLuint texture, GLuint unit) override;
    void bindUniformTexture1D (const char* name, GLuint texture, GLuint unit) override;
    void bindUniformTextureRect (const char* name, GLuint texture, GLuint unit) override;
    void bindUniformTextureArray (const char* name, GLuint texture, GLuint unit) override;

    void bindUniformMatrix (const char* name, const float* m, unsigned int arraySize = 1) override;
    void bindUniformVector (const char* name, const float* m, unsigned int arraySize = 1) override;
    void bindUniformFloat4Array (const char* name, const float* m, unsigned int arraySize = 1) override;
    void bindUniformFloatArray (const char* name, const float* m, unsigned int arraySize = 1) override;
    void bindUniformIntArray (const char* name, const int* iv, unsigned int arraySize) override;

    void bindAttribLocation (GLuint id, const char* name);
};

#endif
