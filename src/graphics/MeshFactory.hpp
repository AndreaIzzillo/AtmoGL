#pragma once

#include <memory>

#include "graphics/Mesh.hpp"

namespace atmogl
{
    class MeshFactory
    {
    public:
        static std::unique_ptr<Mesh> createCube(const glm::vec3 &color = glm::vec3(1.0f));
    };
}
