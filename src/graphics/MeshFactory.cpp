#include "graphics/MeshFactory.hpp"

#include <glm/gtc/noise.hpp>
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

    std::unique_ptr<Mesh> MeshFactory::createTerrain(const glm::vec3 &lowColor,
                                                     const glm::vec3 &highColor)
    {
        constexpr float yRef = 0.0f;
        constexpr int gridSize = 2048;
        constexpr float terrainSize = 1000.0f;
        constexpr float spacing = terrainSize / static_cast<float>(gridSize - 1);
        constexpr float maxAmplitude = 50.0f;

        // Generate vertices
        std::vector<Vertex> vertices(gridSize * gridSize);
        for (int z = 0; z < gridSize; ++z)
        {
            for (int x = 0; x < gridSize; ++x)
            {
                const float xPos = static_cast<float>(x) * spacing - terrainSize / 2.0f;
                const float zPos = static_cast<float>(z) * spacing - terrainSize / 2.0f;

                float height = yRef;
                // Noise
                {
                    constexpr float noiseScale = 300.0f;
                    constexpr int octaves = 6;

                    float amplitude = maxAmplitude;
                    float frequency = 1.0f / noiseScale;

                    for (int i = 0; i < octaves; ++i)
                    {
                        const float n = glm::perlin(glm::vec2(xPos, zPos) * frequency);

                        float ridge = 1.0f - glm::abs(n);
                        height += amplitude * ridge * ridge;

                        frequency *= 2.0;
                        amplitude *= 0.5;
                    }
                }

                vertices[z * gridSize + x].position = glm::vec3(xPos, height, zPos);
                vertices[z * gridSize + x].normal = glm::vec3(0.0f);

                float colorStep = glm::smoothstep(maxAmplitude * 0.5f, maxAmplitude * 2.0f, height);
                vertices[z * gridSize + x].color = glm::mix(lowColor, highColor, colorStep);
            }
        }

        // Generate indices
        std::vector<unsigned int> indices;
        indices.reserve((gridSize - 1) * (gridSize - 1) * 6);
        for (int z = 0; z < gridSize - 1; ++z)
        {
            for (int x = 0; x < gridSize - 1; ++x)
            {
                const unsigned int a = z * gridSize + x;
                const unsigned int b = a + 1;
                const unsigned int c = a + gridSize;
                const unsigned int d = c + 1;

                indices.insert(indices.end(), { a, c, b, b, c, d });
            }
        }

        // Calculate normals
        for (std::size_t i = 0; i < indices.size(); i += 3)
        {
            Vertex &a = vertices[indices[i]];
            Vertex &b = vertices[indices[i + 1]];
            Vertex &c = vertices[indices[i + 2]];

            const glm::vec3 edge1 = b.position - a.position;
            const glm::vec3 edge2 = c.position - a.position;
            const glm::vec3 normal = glm::cross(edge1, edge2);

            a.normal += normal;
            b.normal += normal;
            c.normal += normal;
        }

        // Normalize the normals
        for (Vertex &vertex : vertices)
        {
            vertex.normal = glm::normalize(vertex.normal);
        }

        return std::make_unique<Mesh>(vertices, indices);
    }
}
