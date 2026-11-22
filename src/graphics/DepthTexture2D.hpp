#pragma once

#include <GL/glew.h>

namespace atmogl
{
    class DepthTexture2D
    {
    public:
        DepthTexture2D(int width, int height);
        ~DepthTexture2D();

        DepthTexture2D(const DepthTexture2D &) = delete;
        DepthTexture2D &operator=(const DepthTexture2D &) = delete;

        void resize(int width, int height);
        void bind(GLuint unit) const;

        GLuint getId() const;
        int getWidth() const;
        int getHeight() const;

    private:
        GLuint id = 0;
        int width = 0;
        int height = 0;
    };
}