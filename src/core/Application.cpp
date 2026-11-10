#include "core/Application.hpp"

#include <GL/freeglut.h>
#include <algorithm>

#include "graphics/MeshFactory.hpp"

namespace atmogl
{
    Application *Application::instance = nullptr;

    Application::Application(int &argc, char **argv)
        : window(argc, argv, 1280, 720, "AtmoGL")
        , camera(static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight()))
    {
        renderer.init();
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
        mesh = MeshFactory::createCube(glm::vec3(0.8f, 0.4f, 0.2f));

        objects.emplace_back(*mesh, *shader);
        objects.back().transform.position = glm::vec3(0.0f, 1.0f, 0.0f);
        objects.back().transform.rotation = glm::vec3(0.2f, 0.5f, 0.0f);
        objects.back().transform.scale = glm::vec3(2.0f);
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
        renderer.clear();
        for (const RenderObject &object : objects)
        {
            object.draw(camera);
        }
        window.swapBuffers();
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
        glViewport(0, 0, width, height);
    }

    void Application::closeCallback()
    {
        if (instance)
        {
            instance->objects.clear();
            instance->mesh.reset();
            instance->shader.reset();
            instance->window.notifyClosed();
        }
    }
}
