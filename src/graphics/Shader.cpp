#include "graphics/Shader.hpp"

#include <filesystem>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace atmogl
{
    Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
    {
        const std::string vertexSource = readFile(vertexPath);
        const std::string fragmentSource = readFile(fragmentPath);

        const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
        GLuint fragmentShader = 0;

        try
        {
            fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

            programId = glCreateProgram();
            glAttachShader(programId, vertexShader);
            glAttachShader(programId, fragmentShader);
            linkProgram();
        }
        catch (...)
        {
            glDeleteShader(vertexShader);
            if (fragmentShader != 0)
            {
                glDeleteShader(fragmentShader);
            }
            if (programId != 0)
            {
                glDeleteProgram(programId);
            }
            throw;
        }

        glDetachShader(programId, vertexShader);
        glDetachShader(programId, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::Shader(const std::string &computePath)
    {
        const std::string source = readFile(computePath);
        const GLuint computeShader = compileShader(GL_COMPUTE_SHADER, source, computePath);

        try
        {
            programId = glCreateProgram();
            glAttachShader(programId, computeShader);
            linkProgram();
        }
        catch (...)
        {
            glDeleteShader(computeShader);
            if (programId != 0)
            {
                glDeleteProgram(programId);
            }
            throw;
        }

        glDetachShader(programId, computeShader);
        glDeleteShader(computeShader);
    }

    Shader::~Shader()
    {
        if (programId != 0)
        {
            glDeleteProgram(programId);
        }
    }

    void Shader::use() const
    {
        glUseProgram(programId);
    }

    GLuint Shader::getId() const
    {
        return programId;
    }

    void Shader::setBool(const std::string &name, bool value) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniform1i(location, static_cast<int>(value));
        }
    }

    void Shader::setInt(const std::string &name, int value) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniform1i(location, value);
        }
    }

    void Shader::setFloat(const std::string &name, float value) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniform1f(location, value);
        }
    }

    void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniform3fv(location, 1, glm::value_ptr(value));
        }
    }

    void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniform4fv(location, 1, glm::value_ptr(value));
        }
    }

    void Shader::setMat3(const std::string &name, const glm::mat3 &matrix) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
        }
    }

    void Shader::setMat4(const std::string &name, const glm::mat4 &matrix) const
    {
        const GLint location = glGetUniformLocation(programId, name.c_str());
        if (location != -1)
        {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
        }
    }

    std::string Shader::readFile(const std::string &path, int depth)
    {
        if (depth > 16)
        {
            throw std::runtime_error("Shader include depth exceeded: " + path);
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file: " + path);
        }

        std::ostringstream source;
        std::string line;

        while (std::getline(file, line))
        {
            std::istringstream stream(line);
            std::string directive;
            stream >> directive;

            if (directive == "#include")
            {
                std::string includePath;
                stream >> std::ws;

                if (stream.peek() != '"' || !(stream >> std::quoted(includePath)))
                {
                    throw std::runtime_error("Invalid shader include in: " + path);
                }

                const auto resolvedPath = std::filesystem::path(path).parent_path() / includePath;

                source << readFile(resolvedPath.string(), depth + 1);
            }
            else
            {
                source << line << '\n';
            }
        }

        return source.str();
    }

    GLuint Shader::compileShader(GLenum type, const std::string &source,
                                 const std::string &debugName)
    {
        const GLuint shader = glCreateShader(type);
        const char *sourcePtr = source.c_str();
        glShaderSource(shader, 1, &sourcePtr, nullptr);
        glCompileShader(shader);

        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            std::string log(logLength, '\0');
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            glDeleteShader(shader);

            throw std::runtime_error("Shader compile error in " + debugName + ":\n" + log);
        }

        return shader;
    }

    void Shader::linkProgram()
    {
        glLinkProgram(programId);

        GLint success = GL_FALSE;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);

        if (!success)
        {
            GLint logLength = 0;
            glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);

            std::string log(logLength, '\0');
            glGetProgramInfoLog(programId, logLength, nullptr, log.data());

            throw std::runtime_error("Shader program link error:\n" + log);
        }
    }
}
