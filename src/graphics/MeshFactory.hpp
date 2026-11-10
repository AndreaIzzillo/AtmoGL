#pragma once

#include <memory>

#include "graphics/Mesh.hpp"

namespace atmogl
{
    class MeshFactory
    {
    public:
        static std::unique_ptr<Mesh> createCube(const glm::vec3 &color);
        static std::unique_ptr<Mesh> createQuad();
        static std::unique_ptr<Mesh> createTerrain(const glm::vec3 &lowColor,
                                                   const glm::vec3 &highColor);
    };
}
