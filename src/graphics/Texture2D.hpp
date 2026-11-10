#pragma once

#include <GL/glew.h>

namespace atmogl
{
    class Texture2D
    {
    public:
        Texture2D(int width, int height);
        ~Texture2D();

        Texture2D(const Texture2D &) = delete;
        Texture2D &operator=(const Texture2D &) = delete;

        void resize(int width, int height);
        void bind(GLuint unit) const;
        void bindImage(GLuint unit, GLenum access) const;

        GLuint getId() const;
        int getWidth() const;
        int getHeight() const;

    private:
        GLuint id = 0;
        int width = 0;
        int height = 0;
    };
}
