

#include "Nothing.hpp"

Nothing::Nothing ()
{
}

Nothing::~Nothing ()
{
}

void Nothing::renderLineStrip (int count)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    if (count > 1000000)
        return;
    glBindVertexArray (0);
    glDrawArrays (GL_LINE_STRIP, 0, count);
#endif
}

void Nothing::renderTriangles (int count)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    if (count > 1000000)
        return;
    glBindVertexArray (0);
    glDrawArrays (GL_TRIANGLES, 0, count);
#endif
}

void Nothing::renderPoints (int count)
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    glBindVertexArray (0);
    glDrawArrays (GL_POINTS, 0, count);
#endif
}

void Nothing::renderQuad ()
{
    throw std::runtime_error ("DISABLED CODE");
#if 0
    glBindVertexArray (0);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
#endif
}