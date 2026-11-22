#include "graphics/Framebuffer.hpp"

#include <GL/glew.h>
#include <stdexcept>

namespace atmogl
{
    Framebuffer::~Framebuffer()
    {
        release();
    }

    void Framebuffer::init(int width, int height)
    {
        colorTexture = std::make_unique<Texture2D>(width, height);
        depthTexture = std::make_unique<DepthTexture2D>(width, height);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               colorTexture->getId(), 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                               depthTexture->getId(), 0);

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            throw std::runtime_error("Framebuffer is incomplete");
        }
    }

    void Framebuffer::resize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        colorTexture->resize(width, height);
        depthTexture->resize(width, height);
    }

    void Framebuffer::release()
    {
        if (fbo != 0)
        {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }
        colorTexture.reset();
        depthTexture.reset();
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    const Texture2D &Framebuffer::getColorTexture() const
    {
        return *colorTexture;
    }

    const DepthTexture2D &Framebuffer::getDepthTexture() const
    {
        return *depthTexture;
    }
}
