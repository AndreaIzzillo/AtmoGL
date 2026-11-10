#include "graphics/Renderer.hpp"

#include <GL/glew.h>

namespace atmogl
{
    void Renderer::init()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
    }

    void Renderer::clear() const
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
