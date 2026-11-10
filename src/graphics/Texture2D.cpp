#include "graphics/Texture2D.hpp"

namespace atmogl
{
    Texture2D::Texture2D(int width, int height)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        resize(width, height);
    }

    Texture2D::~Texture2D()
    {
        glDeleteTextures(1, &id);
    }

    void Texture2D::resize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0)
        {
            return;
        }

        width = newWidth;
        height = newHeight;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture2D::bind(GLuint unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void Texture2D::bindImage(GLuint unit, GLenum access) const
    {
        glBindImageTexture(unit, id, 0, GL_FALSE, 0, access, GL_RGBA16F);
    }

    GLuint Texture2D::getId() const
    {
        return id;
    }

    int Texture2D::getWidth() const
    {
        return width;
    }

    int Texture2D::getHeight() const
    {
        return height;
    }
}
