#include <fstream>
#include <iostream>
#include <string.h>

#include "Shader.hpp"

Shader::Shader ()
{
}

Shader::Shader (std::string vertexShaderSource, std::string fragmentShaderSource, bool dummy)
{
    geometryProgram = 0;
    throw std::runtime_error ("DISABLED CODE");
#if 0
  {
	  char* s = new char[vertexShaderSource.length() + 1];
#ifdef _WIN32
	  strncpy_s(s, vertexShaderSource.length()+1, vertexShaderSource.c_str(), vertexShaderSource.length());
#elif __linux__
    strncpy(s, vertexShaderSource.c_str(), vertexShaderSource.length());
#endif
	  shaderFromString(s, vertexShaderSource.length(), GL_VERTEX_SHADER, vertexProgram);
  }
  {
	  char* s = new char[fragmentShaderSource.length() + 1];
#ifdef _WIN32
	  strncpy_s(s, fragmentShaderSource.length()+1, fragmentShaderSource.c_str(), fragmentShaderSource.length());
#elif __linux__
    strncpy(s, fragmentShaderSource.c_str(), fragmentShaderSource.length());
#endif
	  shaderFromString(s, fragmentShaderSource.length(), GL_FRAGMENT_SHADER, fragmentProgram);
  }
  linkShaders(vertexProgram, fragmentProgram, geometryProgram, shaderProgram, GL_LINE_STRIP);
  glDeleteShader(vertexProgram);
  glDeleteShader(fragmentProgram);
#endif
}

Shader::Shader (std::string vertexShaderSource, std::string geometryShaderSource, std::string fragmentShaderSource, GLint geomShaderOutputType)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
	if(vertexShaderSource.empty())
		vertexProgram = 0;
	else
	{
		char* s = new char[vertexShaderSource.length() + 1];
#ifdef _WIN32
		strncpy_s(s, vertexShaderSource.length() + 1, vertexShaderSource.c_str(), vertexShaderSource.length());
#elif __linux__
    strncpy(s, vertexShaderSource.c_str(), vertexShaderSource.length());
#endif
		shaderFromString(s, vertexShaderSource.length(), GL_VERTEX_SHADER, vertexProgram);
	}
	if(geometryShaderSource.empty())
		geometryProgram = 0;
	else
	{
		char* s = new char[geometryShaderSource.length() + 1];
#ifdef _WIN32
		strncpy_s(s, geometryShaderSource.length() + 1, geometryShaderSource.c_str(), geometryShaderSource.length());
#elif __linux__
    strncpy(s, geometryShaderSource.c_str(), geometryShaderSource.length());
#endif
		shaderFromString(s, geometryShaderSource.length(), GL_GEOMETRY_SHADER, geometryProgram);
	}
	if(fragmentShaderSource.empty())
		fragmentProgram = 0;
	else
	{
		char* s = new char[fragmentShaderSource.length() + 1];
#ifdef _WIN32
	  strncpy_s(s, fragmentShaderSource.length()+1, fragmentShaderSource.c_str(), fragmentShaderSource.length());
#elif __linux__
    strncpy(s, fragmentShaderSource.c_str(), fragmentShaderSource.length());
#endif
		shaderFromString(s, fragmentShaderSource.length(), GL_FRAGMENT_SHADER, fragmentProgram);
	}
	linkShaders(vertexProgram, fragmentProgram, geometryProgram, shaderProgram, geomShaderOutputType);
	if(vertexProgram)
		glDeleteShader(vertexProgram);
	if(geometryProgram)
		glDeleteShader(geometryProgram);
	if(fragmentProgram)
		glDeleteShader(fragmentProgram);
#endif
}

Shader::Shader (const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath, GLint geomShaderOutputType)
{
    geometryProgram = 0;
    throw std::runtime_error ("DISABLED CODE");
#if 0
  shaderFromFile(vertexShaderPath, GL_VERTEX_SHADER, vertexProgram);
  shaderFromFile(fragmentShaderPath, GL_FRAGMENT_SHADER, fragmentProgram);
  if(geometryShaderPath)
	  shaderFromFile(geometryShaderPath, GL_GEOMETRY_SHADER, geometryProgram);
  linkShaders(vertexProgram, fragmentProgram, geometryProgram, shaderProgram, geomShaderOutputType);
  glDeleteShader(vertexProgram);
  glDeleteShader(fragmentProgram);
#endif
}

Shader::Shader (std::string vertexShaderPath, std::string fragmentShaderSource)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    geometryProgram = 0;

    shaderFromFile (vertexShaderPath.c_str (), GL_VERTEX_SHADER, vertexProgram);
    char* s = new char[fragmentShaderSource.length () + 1];
#ifdef _WIN32
    strncpy_s (s, fragmentShaderSource.length () + 1, fragmentShaderSource.c_str (), fragmentShaderSource.length ());
#elif __linux__
    strncpy (s, fragmentShaderSource.c_str (), fragmentShaderSource.length ());
#endif
    shaderFromString (s, fragmentShaderSource.length (), GL_FRAGMENT_SHADER, fragmentProgram);
    linkShaders (vertexProgram, fragmentProgram, geometryProgram, shaderProgram, GL_LINE_STRIP);
    glDeleteShader (vertexProgram);
    glDeleteShader (fragmentProgram);
#endif
}

Shader::~Shader ()
{
}

void Shader::shaderFromString (const char* shaderSource, int len, GLenum shaderType, GLuint& handle)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    int errorFlag = -1;

    handle = glCreateShader (shaderType);

    glShaderSource (handle, 1, (const char**)&shaderSource, &len);
    glCompileShader (handle);

    glGetShaderiv (handle, GL_COMPILE_STATUS, &errorFlag);
    if (!errorFlag) {
        std::cout << "=== Shader compile error! ==================" << std::endl;
        if (shaderSource) {
            std::string ssrc (shaderSource, shaderSource + len);
            std::cout << ssrc << std::endl;
        } else
            std::cout << "NO SOURCE CODE!?" << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << getShaderInfoLog (handle) << std::endl;
        exit (-3);
    }

    delete[] shaderSource;
#endif
}

void Shader::shaderFromFile (const char* shaderPath, GLenum shaderType, GLuint& handle)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    char* shaderSource = NULL;
    int   len;

    if (!fileToString (shaderPath, shaderSource, len)) {
        std::cout << "Error loading shader: " << shaderPath << std::endl;
        return;
    }

    shaderFromString (shaderSource, len, shaderType, handle);
#endif
}

void Shader::linkShaders (GLuint& vertexShader, GLuint& fragmentShader, GLuint& geometryShader, GLuint& handle, GLint geomShaderOutputType)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    int errorFlag = -1;

    handle = glCreateProgram ();
    glAttachShader (handle, vertexShader);
    glAttachShader (handle, fragmentShader);
    if (geometryShader != 0) {
        glAttachShader (handle, geometryShader);
        glProgramParameteri (handle, GL_GEOMETRY_VERTICES_OUT_EXT, 4);
        glProgramParameteri (handle, GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
        glProgramParameteri (handle, GL_GEOMETRY_OUTPUT_TYPE_EXT, geomShaderOutputType);
    }

    glLinkProgram (handle);
    glGetProgramiv (handle, GL_LINK_STATUS, &errorFlag);
    if (!errorFlag) {
        std::cout << "Shader link error: " << std::endl;
        std::cout << getProgramInfoLog (handle) << std::endl;
        return;
    }
#endif
}

std::string Shader::getShaderInfoLog (GLuint& object)
{
    int         logLength    = 0;
    int         charsWritten = 0;
    char*       tmpLog;
    std::string log;
    throw std::runtime_error ("DISABLED CODE");
#if 0

    glGetShaderiv (object, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        tmpLog = new char[logLength];
        glGetShaderInfoLog (object, logLength, &charsWritten, tmpLog);
        log = tmpLog;
        delete[] tmpLog;
    }
#endif

    return log;
}

std::string Shader::getProgramInfoLog (GLuint& object)
{
    int         logLength    = 0;
    int         charsWritten = 0;
    char*       tmpLog;
    std::string log;

    throw std::runtime_error ("DISABLED CODE");
#if 0
    glGetProgramiv (object, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0) {
        tmpLog = new char[logLength];
        glGetProgramInfoLog (object, logLength, &charsWritten, tmpLog);
        log = tmpLog;
        delete[] tmpLog;
    }
#endif

    return log;
}
#pragma warning(push)
#pragma warning(disable : 4244)
bool Shader::fileToString (const char* path, char*& out, int& len)
{
    std::ifstream file (path, std::ios::ate | std::ios::binary);
    if (!file.is_open ()) {
        return false;
    }
    len = file.tellg ();
    out = new char[len + 1];
    file.seekg (0, std::ios::beg);
    file.read (out, len);
    file.close ();
    out[len] = 0;
    return true;
}
#pragma warning(pop)

void Shader::enable ()
{
    throw std::runtime_error ("DISABLED CODE");
    // glUseProgram (shaderProgram);
}

void Shader::disable ()
{
    throw std::runtime_error ("DISABLED CODE");
    // glUseProgram (0);
}

void Shader::bindUniformBool (const char* name, bool b)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint boolLocation = glGetUniformLocation (shaderProgram, name);
    //glUniform1f (boolLocation, b);
}

void Shader::bindUniformUint (const char* name, GLuint i)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint vectorLocation = glGetUniformLocation (shaderProgram, name);
    //glUniform1ui (vectorLocation, i);
}

void Shader::bindUniformUint2 (const char* name, GLuint i, GLuint j)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint vectorLocation = glGetUniformLocation (shaderProgram, name);
    //glUniform2ui (vectorLocation, i, j);
}

void Shader::bindUniformInt (const char* name, int i)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint vectorLocation = glGetUniformLocation (shaderProgram, name);
    //glUniform1i (vectorLocation, i);
}

void Shader::bindUniformInt2 (const char* name, int i1, int i2)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint vectorLocation = glGetUniformLocation (shaderProgram, name);
    //glUniform2i (vectorLocation, i1, i2);
}

void Shader::bindUniformFloat (const char* name, float f)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //if (location != -1)
    //    glUniform1f (location, f);
}

void Shader::bindUniformFloat2 (const char* name, float f1, float f2)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    GLuint location = glGetUniformLocation (shaderProgram, name);
    if (location != -1) {
        glUniform2f (location, f1, f2);
        GLenum err  = glGetError ();
        bool   mafv = true;
    }
#endif
}

void Shader::bindUniformFloat3 (const char* name, float f1, float f2, float f3)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //if (location != -1)
    //    glUniform3f (location, f1, f2, f3);
}

void Shader::bindUniformTexture (const char* name, GLuint texture, GLuint unit)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glActiveTexture (GL_TEXTURE0 + unit);
    //glBindTexture (GL_TEXTURE_2D, texture);
    //glUniform1i (location, unit);
}

void Shader::bindUniformTexture1D (const char* name, GLuint texture, GLuint unit)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glActiveTexture (GL_TEXTURE0 + unit);
    //glBindTexture (GL_TEXTURE_1D, texture);
    //glUniform1i (location, unit);
}

void Shader::bindUniformTextureRect (const char* name, GLuint texture, GLuint unit)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glActiveTexture (GL_TEXTURE0 + unit);
    //glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
    //glUniform1i (location, unit);
}

void Shader::bindUniformTextureArray (const char* name, GLuint texture, GLuint unit)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glActiveTexture (GL_TEXTURE0 + unit);
    //glBindTexture (GL_TEXTURE_2D_ARRAY, texture);
    //glUniform1i (location, unit);
}

void Shader::bindAttribLocation (GLuint id, const char* name)
{
    throw std::runtime_error ("DISABLED CODE");
    //glEnableVertexAttribArray (id);
    //glBindAttribLocation (shaderProgram, id, name);
}

void Shader::bindUniformMatrix (const char* name, const float* m, unsigned int arraySize)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glUniformMatrix4fv (location, arraySize, false, m);
}

void Shader::bindUniformVector (const char* name, const float* m, unsigned int arraySize)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glUniform3fv (location, arraySize, m);
}

void Shader::bindUniformFloat4Array (const char* name, const float* m, unsigned int arraySize)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glUniform4fv (location, arraySize, m);
}

void Shader::bindUniformFloatArray (const char* name, const float* m, unsigned int arraySize)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glUniform1fv (location, arraySize, m);
}

void Shader::bindUniformIntArray (const char* name, const int* iv, unsigned int arraySize)
{
    throw std::runtime_error ("DISABLED CODE");
    //GLuint location = glGetUniformLocation (shaderProgram, name);
    //glUniform1iv (location, arraySize, iv);
}
