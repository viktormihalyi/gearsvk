
#include <iostream>

#include "Pointgrid.hpp"

PointGrid::PointGrid (int width, int height)
{
    this->width  = width;
    this->height = height;

    throw std::runtime_error ("DISABLED CODE");
#if 0
    GLfloat* vertices = new GLfloat[3 * width * height];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            vertices[3 * (x + y * width)]     = x / (float)width;  // X
            vertices[3 * (x + y * width) + 1] = y / (float)height; // Y
            vertices[3 * (x + y * width) + 2] = 0.0f;              // Z
        }
    }

    glGenVertexArrays (1, &vertexArray);
    glBindVertexArray (vertexArray);

    GLuint vertexBuffer;
    glGenBuffers (1, &vertexBuffer);
    glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData (GL_ARRAY_BUFFER, sizeof (GLfloat) * 3 * width * height, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer ((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray (0);
#endif
}

PointGrid::~PointGrid ()
{
}

void PointGrid::render (Shader* shader)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    glBindVertexArray (vertexArray);

    shader->bindAttribLocation (0, "position");
    //  shader->bindAttribLocation(1, "texCoord");

    glDrawArrays (GL_POINTS, 0, width * height);

    glBindVertexArray (0);
#endif
}
