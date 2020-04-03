
#ifndef _POINTGRID_
#define _POINTGRID_

#include "GearsSource/OpenGLProxy.hpp"

#include "Shader.hpp"

class PointGrid {
private:
    int width;
    int height;

    GLuint vertexArray;

public:
    PointGrid (int width, int height);
    ~PointGrid ();

    void render (Shader* shader);
};

#endif
