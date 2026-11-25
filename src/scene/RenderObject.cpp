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
        draw(*shader, camera.getViewMatrix(), camera.getProjectionMatrix());
    }

    void RenderObject::draw(const Shader &renderShader, const glm::mat4 &view,
                            const glm::mat4 &projection) const
    {
        renderShader.setMat4("uModel", transform.getMatrix());
        renderShader.setMat4("uView", view);
        renderShader.setMat4("uProjection", projection);
        mesh->draw();
    }
}
