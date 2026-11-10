#pragma once

#include "scene/Transform.hpp"

namespace atmogl
{
    class Camera;
    class Mesh;
    class Shader;

    class RenderObject
    {
    public:
        RenderObject(const Mesh &mesh, const Shader &shader);
        void draw(const Camera &camera) const;

        Transform transform;

    private:
        const Mesh *mesh;
        const Shader *shader;
    };
}
