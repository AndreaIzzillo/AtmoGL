#include "scene/RenderObject.hpp"

#include "graphics/Mesh.hpp"
#include "graphics/Shader.hpp"
#include "scene/Camera.hpp"

namespace atmogl
{
    RenderObject::RenderObject(const Mesh &mesh, const Shader &shader)
        : mesh(&mesh)
        , shader(&shader)
    {}

    void RenderObject::draw(const Camera &camera) const
    {
        shader->use();
        shader->setMat4("uModel", transform.getMatrix());
        shader->setMat4("uView", camera.getViewMatrix());
        shader->setMat4("uProjection", camera.getProjectionMatrix());
        mesh->draw();
    }
}
