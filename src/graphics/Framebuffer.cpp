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

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               colorTexture->getId(), 0);

        glGenRenderbuffers(1, &depthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
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
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void Framebuffer::release()
    {
        if (fbo != 0)
        {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }
        if (depthRbo != 0)
        {
            glDeleteRenderbuffers(1, &depthRbo);
            depthRbo = 0;
        }
        colorTexture.reset();
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    const Texture2D &Framebuffer::getColorTexture() const
    {
        return *colorTexture;
    }
}
