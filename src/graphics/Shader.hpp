#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

namespace atmogl
{
    class Shader
    {
    public:
        Shader(const std::string &vertexPath, const std::string &fragmentPath);
        explicit Shader(const std::string &computePath);
        ~Shader();

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        void use() const;
        GLuint getId() const;

        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec4(const std::string &name, const glm::vec4 &value) const;
        void setMat3(const std::string &name, const glm::mat3 &matrix) const;
        void setMat4(const std::string &name, const glm::mat4 &matrix) const;

    private:
        GLuint programId = 0;

        static std::string readFile(const std::string &path);
        static GLuint compileShader(GLenum type, const std::string &source,
                                    const std::string &debugName);
        void linkProgram();
    };
}
