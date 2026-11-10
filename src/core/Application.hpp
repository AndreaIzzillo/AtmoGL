#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <vector>

#include "core/Window.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Shader.hpp"
#include "scene/Camera.hpp"
#include "scene/RenderObject.hpp"

namespace atmogl
{
    class Application
    {
    public:
        Application(int &argc, char **argv);
        ~Application();

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        void run();
        void update(float dt);
        void render();

    private:
        Window window;
        Framebuffer sceneFramebuffer;
        Camera camera;

        std::unique_ptr<Shader> shader;
        std::unique_ptr<Shader> computeShader;
        std::unique_ptr<Shader> screenShader;
        std::unique_ptr<Mesh> terrain;
        std::unique_ptr<Mesh> screenQuad;
        std::vector<RenderObject> objects;

        std::array<bool, 256> keys{};
        std::chrono::steady_clock::time_point previousTime;
        bool mouseCaptured = false;
        int lastMouseX = 0;
        int lastMouseY = 0;

        static Application *instance;

        void loadScene();
        void renderScene();
        void processScene();
        void presentScene();

        static void displayCallback();
        static void idleCallback();
        static void keyboardDownCallback(unsigned char key, int x, int y);
        static void keyboardUpCallback(unsigned char key, int x, int y);
        static void mouseButtonCallback(int button, int state, int x, int y);
        static void mouseMotionCallback(int x, int y);
        static void reshapeCallback(int width, int height);
        static void closeCallback();
    };
}
