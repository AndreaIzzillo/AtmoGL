#include "core/Application.hpp"

#include <GL/freeglut.h>
#include <algorithm>
#include <glm/gtc/color_space.hpp>

#include "graphics/MeshFactory.hpp"

namespace atmogl
{
    Application *Application::instance = nullptr;

    Application::Application(int &argc, char **argv)
        : window(argc, argv, 1280, 720, "AtmoGL")
        , camera(static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight()))
    {
        sceneFramebuffer.init(window.getWidth(), window.getHeight());
        loadScene();

        instance = this;
        previousTime = std::chrono::steady_clock::now();

        glutDisplayFunc(displayCallback);
        glutIdleFunc(idleCallback);
        glutKeyboardFunc(keyboardDownCallback);
        glutKeyboardUpFunc(keyboardUpCallback);
        glutMouseFunc(mouseButtonCallback);
        glutMotionFunc(mouseMotionCallback);
        glutReshapeFunc(reshapeCallback);
        glutCloseFunc(closeCallback);
    }

    Application::~Application()
    {
        instance = nullptr;
    }

    void Application::loadScene()
    {
        shader = std::make_unique<Shader>("shaders/mesh.vert", "shaders/mesh.frag");
        computeShader = std::make_unique<Shader>("shaders/invert.comp");
        screenShader = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");

        const auto lowColor = glm::convertSRGBToLinear(glm::vec3(0.5f, 0.2f, 0.2f));
        const auto highColor = glm::convertSRGBToLinear(glm::vec3(0.8f, 0.3f, 0.0f));
        terrain = MeshFactory::createTerrain(lowColor, highColor);

        screenQuad = MeshFactory::createQuad();

        objects.emplace_back(*terrain, *shader);
    }

    void Application::run()
    {
        window.startMainLoop();
    }

    void Application::update(float dt)
    {
        if (keys[27])
            glutLeaveMainLoop();
        if (keys['w'])
            camera.moveForward(dt);
        if (keys['s'])
            camera.moveBackward(dt);
        if (keys['a'])
            camera.moveLeft(dt);
        if (keys['d'])
            camera.moveRight(dt);
        if (keys['q'])
            camera.moveUp(dt);
        if (keys['e'])
            camera.moveDown(dt);
        if (keys['j'])
            camera.rotateLeft(dt);
        if (keys['l'])
            camera.rotateRight(dt);
        if (keys['i'])
            camera.rotateUp(dt);
        if (keys['k'])
            camera.rotateDown(dt);
        if (keys['c'])
            camera.increaseSpeed(dt);
        if (keys['x'])
            camera.decreaseSpeed(dt);
    }

    void Application::render()
    {
        renderScene();
        processScene();
        presentScene();

        window.swapBuffers();
    }

    void Application::renderScene()
    {
        sceneFramebuffer.bind();

        const auto clearColor = glm::convertSRGBToLinear(glm::vec3(0.08f, 0.10f, 0.14f));
        const Texture2D &texture = sceneFramebuffer.getColorTexture();
        glViewport(0, 0, texture.getWidth(), texture.getHeight());
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_BLEND);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const RenderObject &object : objects)
        {
            object.draw(camera);
        }
    }

    void Application::processScene()
    {
        const Texture2D &texture = sceneFramebuffer.getColorTexture();
        texture.bindImage(0, GL_READ_WRITE);

        computeShader->use();
        glDispatchCompute((texture.getWidth() + 7) / 8, (texture.getHeight() + 7) / 8, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
    }

    void Application::presentScene()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, window.getWidth(), window.getHeight());
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_FRAMEBUFFER_SRGB);

        sceneFramebuffer.getColorTexture().bind(0);
        screenShader->use();
        screenShader->setFloat("uExposure", exposure);
        screenQuad->draw();
    }

    void Application::displayCallback()
    {
        if (instance)
        {
            instance->render();
        }
    }

    void Application::idleCallback()
    {
        if (!instance)
        {
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        const float dt =
            std::min(std::chrono::duration<float>(now - instance->previousTime).count(), 0.1f);
        instance->previousTime = now;

        instance->update(dt);
        glutPostRedisplay();
    }

    void Application::keyboardDownCallback(unsigned char key, int, int)
    {
        if (instance)
        {
            instance->keys[key] = true;
        }
    }

    void Application::keyboardUpCallback(unsigned char key, int, int)
    {
        if (instance)
        {
            instance->keys[key] = false;
        }
    }

    void Application::mouseButtonCallback(int button, int state, int x, int y)
    {
        if (!instance || button != GLUT_LEFT_BUTTON)
        {
            return;
        }

        instance->mouseCaptured = state == GLUT_DOWN;
        instance->lastMouseX = x;
        instance->lastMouseY = y;
        glutSetCursor(instance->mouseCaptured ? GLUT_CURSOR_NONE : GLUT_CURSOR_INHERIT);
    }

    void Application::mouseMotionCallback(int x, int y)
    {
        if (!instance || !instance->mouseCaptured)
        {
            return;
        }

        instance->camera.rotateByMouse(static_cast<float>(x - instance->lastMouseX),
                                       static_cast<float>(y - instance->lastMouseY));
        instance->lastMouseX = x;
        instance->lastMouseY = y;
    }

    void Application::reshapeCallback(int width, int height)
    {
        if (!instance || width <= 0 || height <= 0)
        {
            return;
        }

        instance->window.setSize(width, height);
        instance->camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
        instance->sceneFramebuffer.resize(width, height);

        glViewport(0, 0, width, height);
    }

    void Application::closeCallback()
    {
        if (instance)
        {
            instance->objects.clear();
            instance->screenQuad.reset();
            instance->terrain.reset();
            instance->screenShader.reset();
            instance->computeShader.reset();
            instance->shader.reset();
            instance->sceneFramebuffer.release();
            instance->window.notifyClosed();
        }
    }
}
