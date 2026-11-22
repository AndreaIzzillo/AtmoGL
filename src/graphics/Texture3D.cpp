#include "graphics/Texture3D.hpp"

namespace atmogl
{
    Texture3D::Texture3D(int width, int height, int depth)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        resize(width, height, depth);
    }

    Texture3D::~Texture3D()
    {
        glDeleteTextures(1, &id);
    }

    void Texture3D::resize(int newWidth, int newHeight, int newDepth)
    {
        if (newWidth <= 0 || newHeight <= 0 || newDepth <= 0)
        {
            return;
        }

        width = newWidth;
        height = newHeight;
        depth = newDepth;
        glBindTexture(GL_TEXTURE_3D, id);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT,
                     nullptr);
        glBindTexture(GL_TEXTURE_3D, 0);
    }

    void Texture3D::bind(GLuint unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_3D, id);
    }

    void Texture3D::bindImage(GLuint unit, GLenum access) const
    {
        glBindImageTexture(unit, id, 0, GL_TRUE, 0, access, GL_RGBA16F);
    }

    GLuint Texture3D::getId() const
    {
        return id;
    }

    int Texture3D::getWidth() const
    {
        return width;
    }

    int Texture3D::getHeight() const
    {
        return height;
    }

    int Texture3D::getDepth() const
    {
        return depth;
    }
}
