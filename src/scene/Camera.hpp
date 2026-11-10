#pragma once

#include <glm/glm.hpp>

namespace atmogl
{
    class Camera
    {
    public:
        explicit Camera(float aspectRatio);
        ~Camera() = default;

        glm::vec3 getPosition() const;
        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix() const;

        void setAspectRatio(float aspectRatio);

        void setPosition(const glm::vec3 &newPosition);
        void movePosition(const glm::vec3 &delta);
        void moveForward(float dt);
        void moveBackward(float dt);
        void moveLeft(float dt);
        void moveRight(float dt);
        void moveUp(float dt);
        void moveDown(float dt);
        void rotateLeft(float dt);
        void rotateRight(float dt);
        void rotateUp(float dt);
        void rotateDown(float dt);
        void rotateByMouse(float dx, float dy);
        void increaseSpeed(float dt);
        void decreaseSpeed(float dt);

    private:
        void rotate(float yawDelta, float pitchDelta);

        glm::vec3 position = { 0.0f, 1.5f, 6.0f };
        glm::vec3 front = { 0.0f, 0.0f, -1.0f };
        glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
        glm::vec3 up = { 0.0f, 1.0f, 0.0f };
        glm::vec3 right = { 1.0f, 0.0f, 0.0f };

        float movementSpeed = 5.0f;
        float rotationSpeed = 90.0f;
        float mouseSensitivity = 0.1f;

        float fov = 60.0f;
        float aspectRatio = 1.0f;
        float nearPlane = 0.1f;
        float farPlane = 500.0f;
    };
}
