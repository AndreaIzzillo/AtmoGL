#include "graphics/MeshFactory.hpp"

#include <vector>

namespace atmogl
{
    std::unique_ptr<Mesh> MeshFactory::createCube(const glm::vec3 &color)
    {
        std::vector<Vertex> vertices = {
            // Front face
            { { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, color },
            { { 0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, color },
            { { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, color },
            { { -0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, color },
            // Back face
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, color },
            { { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, color },
            { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, color },
            { { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, color },
            // Left face
            { { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, color },
            { { -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, color },
            { { -0.5f, 0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, color },
            { { -0.5f, 0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, color },
            // Right face
            { { 0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, color },
            { { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, color },
            { { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, color },
            { { 0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, color },
            // Top face
            { { -0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, color },
            { { -0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, color },
            { { 0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, color },
            { { 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, color },
            // Bottom face
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, color },
            { { 0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, color },
            { { 0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, color },
            { { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, color },
        };

        std::vector<unsigned int> indices = {
            0,  1,  2,  0,  2,  3, // Front face
            4,  5,  6,  4,  6,  7, // Back face
            8,  9,  10, 8,  10, 11, // Left face
            12, 13, 14, 12, 14, 15, // Right face
            16, 17, 18, 16, 18, 19, // Top face
            20, 21, 22, 20, 22, 23 // Bottom face
        };

        return std::make_unique<Mesh>(vertices, indices);
    }

    std::unique_ptr<Mesh> MeshFactory::createQuad()
    {
        const glm::vec3 normal(0.0f, 0.0f, 1.0f);
        const glm::vec3 color(1.0f);
        const std::vector<Vertex> vertices = { { { -1.0f, -1.0f, 0.0f }, normal, color },
                                               { { 1.0f, -1.0f, 0.0f }, normal, color },
                                               { { 1.0f, 1.0f, 0.0f }, normal, color },
                                               { { -1.0f, 1.0f, 0.0f }, normal, color } };
        const std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

        return std::make_unique<Mesh>(vertices, indices);
    }
}
