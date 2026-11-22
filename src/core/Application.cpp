#include "core/Application.hpp"

#include <GL/freeglut.h>
#include <algorithm>
#include <glm/gtc/color_space.hpp>
#include <imgui.h>
#include <imgui_impl_glut.h>

#include "graphics/MeshFactory.hpp"

#define ESC 27

namespace atmogl
{
    Application *Application::instance = nullptr;

    Application::Application(int &argc, char **argv)
        : window(argc, argv, 1920, 1080, "AtmoGL")
        , camera(static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight()))
    {
        sceneFramebuffer.init(window.getWidth(), window.getHeight());
        loadScene();
        gui = std::make_unique<GUI>();
        atmosphere = std::make_unique<Atmosphere>();
        atmosphere->computeTransmittance(atmosphereParameters);

        instance = this;
        previousTime = std::chrono::steady_clock::now();

        glutDisplayFunc(displayCallback);
        glutIdleFunc(idleCallback);
        glutKeyboardFunc(keyboardDownCallback);
        glutKeyboardUpFunc(keyboardUpCallback);
        glutSpecialFunc(specialDownCallback);
        glutSpecialUpFunc(specialUpCallback);
        glutMouseFunc(mouseButtonCallback);
        glutMotionFunc(mouseMotionCallback);
        glutPassiveMotionFunc(mouseMotionCallback);
        glutMouseWheelFunc(mouseWheelCallback);
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
        finalShader = std::make_unique<Shader>("shaders/final.comp");
        screenMeshShader =
            std::make_unique<Shader>("shaders/screen.vert", "shaders/screen_mesh.frag");
        screenSkyShader =
            std::make_unique<Shader>("shaders/screen.vert", "shaders/screen_sky.frag");

        const auto lowColor = glm::convertSRGBToLinear(glm::vec3(0.24f, 0.32f, 0.16f));
        const auto highColor = glm::convertSRGBToLinear(glm::vec3(0.42f, 0.40f, 0.37f));
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
        if (gui && gui->wantsKeyboard())
            return;

        if (keys[ESC])
        {
            glutLeaveMainLoop();
            return;
        }

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
        gui->beginFrame();
        drawGui();

        glm::vec3 cameraPosition =
            camera.getPosition() + glm::vec3(0.0f, atmosphereParameters.groundRadius, 0.0f);
        atmosphere->computeSkyView(atmosphereParameters, cameraPosition);

        // Render passes
        {
            renderScene();
            presentScene();
            gui->render();
        }

        window.swapBuffers();
    }

    void Application::drawGui()
    {
        ImGui::Begin("AtmoGL");

        if (ImGui::CollapsingHeader("Display Mode"))
        {
            if (ImGui::RadioButton("Scene", displayMode == DisplayMode::Scene))
                displayMode = DisplayMode::Scene;
            if (ImGui::RadioButton("Transmittance LUT",
                                   displayMode == DisplayMode::TransmittanceLut))
                displayMode = DisplayMode::TransmittanceLut;
            if (ImGui::RadioButton("Sky View LUT", displayMode == DisplayMode::SkyViewLut))
                displayMode = DisplayMode::SkyViewLut;
            if (ImGui::RadioButton("Distance", displayMode == DisplayMode::Distance))
                displayMode = DisplayMode::Distance;
        }

        if (ImGui::CollapsingHeader("Atmosphere Parameters"))
        {
            ImGui::Checkbox("Aerial Perspective", &aerialPerspectiveEnabled);
            ImGui::SliderFloat("Sun Elevation", &atmosphereParameters.sunElevation, -90.0f, 90.0f,
                               "%.1f");
            ImGui::SliderFloat("Sun Azimuth", &atmosphereParameters.sunAzimuth, 0.0f, 360.0f,
                               "%.1f");
            ImGui::ColorEdit3("Sun Illuminance", &atmosphereParameters.sunIlluminance[0]);
            ImGui::SliderFloat("Sun Angular Radius", &atmosphereParameters.sunAngularRadius, 0.0f,
                               1.0f, "%.3f");
        }

        if (ImGui::CollapsingHeader("HDR Settings"))
        {
            ImGui::SliderFloat("Exposition", &exposure, 0.01f, 10.0f, "%.2f");
        }

        ImGui::End();
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

        // Sky pass
        {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);

            const glm::vec3 cameraPosition =
                camera.getPosition() + glm::vec3(0.0f, atmosphereParameters.groundRadius, 0.0f);

            screenSkyShader->use();
            screenSkyShader->setMat4("uInverseProjection",
                                     glm::inverse(camera.getProjectionMatrix()));
            screenSkyShader->setMat4("uInverseView", glm::inverse(camera.getViewMatrix()));
            screenSkyShader->setVec3("uCameraPosition", cameraPosition);
            screenSkyShader->setFloat("uGroundRadius", atmosphereParameters.groundRadius);
            screenSkyShader->setVec3("uSunDirection", atmosphereParameters.sunDirection());
            screenSkyShader->setVec3("uSunIlluminance", atmosphereParameters.sunIlluminance);
            screenSkyShader->setFloat("uSunAngularRadius", atmosphereParameters.sunAngularRadius);
            screenSkyShader->setFloat("uTopRadius", atmosphereParameters.AtmosphereRadius);

            atmosphere->getTransmittanceLut().bind(0);
            atmosphere->getSkyViewLut().bind(1);

            screenQuad->draw();

            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }

        // Mesh pass
        {
            shader->use();
            shader->setVec3("uSunDirection", atmosphereParameters.sunDirection());
            shader->setVec3("uSunIlluminance", atmosphereParameters.sunIlluminance);
            shader->setFloat("uGroundRadius", atmosphereParameters.groundRadius);
            shader->setFloat("uTopRadius", atmosphereParameters.AtmosphereRadius);

            atmosphere->getTransmittanceLut().bind(0);

            for (const RenderObject &object : objects)
            {
                object.draw(camera);
            }
        }
    }

    void Application::processScene()
    {
        const Texture2D &texture = sceneFramebuffer.getColorTexture();
        texture.bindImage(3, GL_READ_WRITE);

        finalShader->use();
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

        atmosphere->getTransmittanceLut().bind(0);
        atmosphere->getSkyViewLut().bind(1);
        sceneFramebuffer.getColorTexture().bind(3);
        sceneFramebuffer.getDepthTexture().bind(4);

        screenMeshShader->use();
        atmosphere->setCommonUniforms(*screenMeshShader, atmosphereParameters);
        screenMeshShader->setBool("uEnableAerialPerspective",
                                  aerialPerspectiveEnabled && displayMode == DisplayMode::Scene);
        screenMeshShader->setMat4("uInverseView", glm::inverse(camera.getViewMatrix()));
        screenMeshShader->setVec3("uCameraPosition",
                                  camera.getPosition()
                                      + glm::vec3(0.0f, atmosphereParameters.groundRadius, 0.0f));
        screenMeshShader->setVec3("uSunDirection", atmosphereParameters.sunDirection());
        screenMeshShader->setVec3("uSunIlluminance", atmosphereParameters.sunIlluminance);
        screenMeshShader->setFloat("uMieAsymmetry", atmosphereParameters.mieAsymmetry);
        screenMeshShader->setBool("uShowDistance", displayMode == DisplayMode::Distance);
        screenMeshShader->setMat4("uInverseProjection", glm::inverse(camera.getProjectionMatrix()));
        screenMeshShader->setFloat("uDistanceMax", 20000.0f);
        screenMeshShader->setFloat("uExposure", exposure);
        screenMeshShader->setBool("uShowTransmittanceLut",
                                  displayMode == DisplayMode::TransmittanceLut);
        screenMeshShader->setBool("uShowSkyViewLut", displayMode == DisplayMode::SkyViewLut);
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

    void Application::keyboardDownCallback(unsigned char key, int x, int y)
    {
        if (instance)
        {
            if (instance->gui)
                ImGui_ImplGLUT_KeyboardFunc(key, x, y);
            instance->keys[key] = true;
        }
    }

    void Application::keyboardUpCallback(unsigned char key, int x, int y)
    {
        if (instance)
        {
            if (instance->gui)
                ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);
            instance->keys[key] = false;
        }
    }

    void Application::specialDownCallback(int key, int x, int y)
    {
        if (instance && instance->gui)
            ImGui_ImplGLUT_SpecialFunc(key, x, y);
    }

    void Application::specialUpCallback(int key, int x, int y)
    {
        if (instance && instance->gui)
            ImGui_ImplGLUT_SpecialUpFunc(key, x, y);
    }

    void Application::mouseButtonCallback(int button, int state, int x, int y)
    {
        if (!instance)
            return;

        if (instance->gui)
            ImGui_ImplGLUT_MouseFunc(button, state, x, y);

        if (button != GLUT_LEFT_BUTTON)
            return;

        if (state == GLUT_UP)
        {
            instance->mouseCaptured = false;
            glutSetCursor(GLUT_CURSOR_INHERIT);
            return;
        }

        if (instance->gui && instance->gui->wantsMouse())
            return;

        instance->mouseCaptured = true;
        instance->lastMouseX = x;
        instance->lastMouseY = y;
        glutSetCursor(GLUT_CURSOR_NONE);
    }

    void Application::mouseWheelCallback(int button, int direction, int x, int y)
    {
        if (instance && instance->gui)
            ImGui_ImplGLUT_MouseWheelFunc(button, direction, x, y);
    }

    void Application::mouseMotionCallback(int x, int y)
    {
        if (!instance)
            return;

        if (instance->gui)
            ImGui_ImplGLUT_MotionFunc(x, y);

        if (!instance->mouseCaptured || (instance->gui && instance->gui->wantsMouse()))
            return;

        instance->camera.rotateByMouse(static_cast<float>(x - instance->lastMouseX),
                                       static_cast<float>(y - instance->lastMouseY));
        instance->lastMouseX = x;
        instance->lastMouseY = y;
    }

    void Application::reshapeCallback(int width, int height)
    {
        if (!instance)
            return;

        if (instance->gui)
            ImGui_ImplGLUT_ReshapeFunc(width, height);

        if (width <= 0 || height <= 0)
            return;

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
            instance->gui.reset();
            instance->objects.clear();
            instance->screenQuad.reset();
            instance->terrain.reset();
            instance->screenMeshShader.reset();
            instance->screenSkyShader.reset();
            instance->finalShader.reset();
            instance->shader.reset();
            instance->sceneFramebuffer.release();
            instance->window.notifyClosed();
            instance->atmosphere.reset();
        }
    }
}
