

#include "Quad.hpp"

#include <exception>

GLfloat Quad::vertices[18] = {-1.0f, 1.0f, 0.0f,
                              1.0f, 1.0f, 0.0f,
                              -1.0f, -1.0f, 0.0f,
                              -1.0f, -1.0f, 0.0f,
                              1.0f, 1.0f, 0.0f,
                              1.0f, -1.0f, 0.0f};

GLfloat Quad::texCoords[12] = {0.0f, 0.0f,
                               1.0f, 0.0f,
                               0.0f, 1.0f,
                               0.0f, 1.0f,
                               1.0f, 0.0f,
                               1.0f, 1.0f};

Quad::Quad ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 18, vertices, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glGenBuffers(1, &texCoordBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, texCoords, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
#endif
}

void Quad::render (Shader* shader)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    glBindVertexArray (vertexArray);

    shader->bindAttribLocation (0, "position");
    shader->bindAttribLocation (1, "texCoord");

    glDrawArrays (GL_TRIANGLES, 0, 6);

    glBindVertexArray (0);
#endif
}
