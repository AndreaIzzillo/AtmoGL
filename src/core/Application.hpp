#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <vector>

#include "atmosphere/Atmosphere.hpp"
#include "core/Window.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Shader.hpp"
#include "scene/Camera.hpp"
#include "scene/RenderObject.hpp"
#include "ui/GUI.hpp"

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
        Framebuffer sunFramebuffer;
        Camera camera;

        static constexpr float sunNearPlane = 1.0f;
        static constexpr float sunFarPlane = 120'000.0f;
        glm::mat4 lightViewProjection = glm::mat4(1.0f);
        glm::vec3 lastSunDirection = glm::vec3(0.0f);
        float lastShadowSlopeBias = 0.0f;
        bool sunViewValid = false;

        float exposure = 1.75f;
        float shadowBias = 48.0f;
        float shadowSlopeBias = 2.0f;
        float aerialPerspectiveDebugDepth = 0.0f;
        bool skyViewEnabled = true;
        bool aerialPerspectiveEnabled = true;
        bool useAerialPerspectiveLut = true;
        bool transmittanceDirty = false;
        AtmosphereParameters atmosphereParameters;

        std::unique_ptr<GUI> gui;
        std::unique_ptr<Shader> shader;
        std::unique_ptr<Shader> finalShader;
        std::unique_ptr<Shader> screenMeshShader;
        std::unique_ptr<Shader> screenSkyShader;
        std::unique_ptr<Shader> sunViewShader;
        std::unique_ptr<Mesh> terrain;
        std::unique_ptr<Mesh> screenQuad;
        std::vector<RenderObject> objects;
        std::unique_ptr<Atmosphere> atmosphere;

        std::array<bool, 256> keys{};
        std::chrono::steady_clock::time_point previousTime;
        bool mouseCaptured = false;
        int lastMouseX = 0;
        int lastMouseY = 0;

        static Application *instance;

        void loadScene();
        void renderScene();
        void renderSunView();
        void processScene();
        void presentScene();
        void drawGui();

        static void displayCallback();
        static void idleCallback();
        static void keyboardDownCallback(unsigned char key, int x, int y);
        static void keyboardUpCallback(unsigned char key, int x, int y);
        static void specialDownCallback(int key, int x, int y);
        static void specialUpCallback(int key, int x, int y);
        static void mouseButtonCallback(int button, int state, int x, int y);
        static void mouseWheelCallback(int button, int direction, int x, int y);
        static void mouseMotionCallback(int x, int y);
        static void reshapeCallback(int width, int height);
        static void closeCallback();

        enum class DisplayMode
        {
            Scene,
            TransmittanceLut,
            SkyViewLut,
            AerialPerspectiveLut,
            Distance,
            SunView
        };

        DisplayMode displayMode = DisplayMode::Scene;
    };
}
