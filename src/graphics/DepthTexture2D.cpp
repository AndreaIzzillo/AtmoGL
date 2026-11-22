#include "graphics/DepthTexture2D.hpp"

namespace atmogl
{
    DepthTexture2D::DepthTexture2D(int width, int height)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        resize(width, height);
    }

    DepthTexture2D::~DepthTexture2D()
    {
        glDeleteTextures(1, &id);
    }

    void DepthTexture2D::resize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0)
        {
            return;
        }

        width = newWidth;
        height = newHeight;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void DepthTexture2D::bind(GLuint unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    GLuint DepthTexture2D::getId() const
    {
        return id;
    }

    int DepthTexture2D::getWidth() const
    {
        return width;
    }

    int DepthTexture2D::getHeight() const
    {
        return height;
    }
}
