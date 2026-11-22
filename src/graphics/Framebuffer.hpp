#pragma once

#include <memory>

#include "graphics/DepthTexture2D.hpp"
#include "graphics/Texture2D.hpp"

namespace atmogl
{
    class Framebuffer
    {
    public:
        Framebuffer() = default;
        ~Framebuffer();

        Framebuffer(const Framebuffer &) = delete;
        Framebuffer &operator=(const Framebuffer &) = delete;

        void init(int width, int height);
        void resize(int width, int height);
        void release();

        void bind() const;

        const Texture2D &getColorTexture() const;
        const DepthTexture2D &getDepthTexture() const;

    private:
        GLuint fbo = 0;
        std::unique_ptr<Texture2D> colorTexture;
        std::unique_ptr<DepthTexture2D> depthTexture;
    };
}
