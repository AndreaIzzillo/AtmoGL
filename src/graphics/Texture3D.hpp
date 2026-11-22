#pragma once

#include <GL/glew.h>

namespace atmogl
{
    class Texture3D
    {
    public:
        Texture3D(int width, int height, int depth);
        ~Texture3D();

        Texture3D(const Texture3D &) = delete;
        Texture3D &operator=(const Texture3D &) = delete;

        void resize(int width, int height, int depth);
        void bind(GLuint unit) const;
        void bindImage(GLuint unit, GLenum access) const;

        GLuint getId() const;
        int getWidth() const;
        int getHeight() const;
        int getDepth() const;

    private:
        GLuint id = 0;
        int width = 0;
        int height = 0;
        int depth = 0;
    };
}
