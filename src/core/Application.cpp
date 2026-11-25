#include "core/Application.hpp"

#include <GL/freeglut.h>
#include <algorithm>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
        sunFramebuffer.init(2 << 12, 2 << 12);

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
        sunViewShader = std::make_unique<Shader>("shaders/mesh.vert", "shaders/sun_view.frag");

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

        if (transmittanceDirty)
        {
            atmosphere->computeTransmittance(atmosphereParameters);
            transmittanceDirty = false;
        }

        renderSunView();

        if (displayMode != DisplayMode::SunView)
        {
            const glm::vec3 cameraPosition =
                camera.getPosition() + glm::vec3(0.0f, atmosphereParameters.groundRadius, 0.0f);
            const float normalizedShadowBias = shadowBias / (sunFarPlane - sunNearPlane);
            sunFramebuffer.getDepthTexture().bind(6);
            atmosphere->computeSkyView(atmosphereParameters, cameraPosition);

            if (displayMode == DisplayMode::AerialPerspectiveLut
                || (aerialPerspectiveEnabled && useAerialPerspectiveLut
                    && displayMode == DisplayMode::Scene))
            {
                atmosphere->computeAerialPerspective(atmosphereParameters, cameraPosition,
                                                     glm::inverse(camera.getProjectionMatrix()),
                                                     glm::inverse(camera.getViewMatrix()),
                                                     camera.getFarPlane(), lightViewProjection,
                                                     normalizedShadowBias);
            }

            renderScene();
        }

        presentScene();
        gui->render();
        window.swapBuffers();
    }

    void Application::drawGui()
    {
        ImGui::Begin("AtmoGL");

        if (ImGui::CollapsingHeader("Display Mode"))
        {
            if (ImGui::RadioButton("Scene", displayMode == DisplayMode::Scene))
            {
                displayMode = DisplayMode::Scene;
            }
            if (ImGui::RadioButton("Transmittance LUT",
                                   displayMode == DisplayMode::TransmittanceLut))
            {
                displayMode = DisplayMode::TransmittanceLut;
            }
            if (ImGui::RadioButton("Sky View LUT", displayMode == DisplayMode::SkyViewLut))
            {
                displayMode = DisplayMode::SkyViewLut;
            }
            if (ImGui::RadioButton("Aerial Perspective LUT",
                                   displayMode == DisplayMode::AerialPerspectiveLut))
            {
                displayMode = DisplayMode::AerialPerspectiveLut;
            }
            if (displayMode == DisplayMode::AerialPerspectiveLut)
            {
                ImGui::SliderFloat("Depth (m)", &aerialPerspectiveDebugDepth, 0.0f,
                                   camera.getFarPlane(), "%.1f");
            }
            if (ImGui::RadioButton("Distance", displayMode == DisplayMode::Distance))
            {
                displayMode = DisplayMode::Distance;
            }
            if (ImGui::RadioButton("Sun View", displayMode == DisplayMode::SunView))
            {
                displayMode = DisplayMode::SunView;
            }
        }

        if (ImGui::CollapsingHeader("Atmosphere Parameters"))
        {
            ImGui::Checkbox("Sky View", &skyViewEnabled);
            ImGui::Checkbox("Aerial Perspective", &aerialPerspectiveEnabled);
            if (aerialPerspectiveEnabled)
                ImGui::Checkbox("Use AP LUT", &useAerialPerspectiveLut);
            ImGui::SliderFloat("Sun Elevation", &atmosphereParameters.sunElevation, -90.0f, 90.0f,
                               "%.1f");
            ImGui::SliderFloat("Sun Azimuth", &atmosphereParameters.sunAzimuth, 0.0f, 360.0f,
                               "%.1f");
            ImGui::ColorEdit3("Sun Illuminance", &atmosphereParameters.sunIlluminance[0]);
            ImGui::SliderFloat("Sun Angular Radius", &atmosphereParameters.sunAngularRadius, 0.0f,
                               1.0f, "%.3f");

            const auto editCoefficient = [](const char *label, glm::vec3 &coefficient) {
                glm::vec3 value = coefficient * 1e6f;
                if (ImGui::DragFloat3(label, &value[0], 0.01f, 0.0f, 1000.0f, "%.3f",
                                      ImGuiSliderFlags_AlwaysClamp))
                {
                    coefficient = value * 1e-6f;
                    return true;
                }
                return false;
            };

            if (ImGui::TreeNode("Rayleigh"))
            {
                transmittanceDirty |= editCoefficient("Rayleigh Scattering (1e-6 m^-1)",
                                                      atmosphereParameters.rayleighScattering);
                transmittanceDirty |= ImGui::DragFloat(
                    "Rayleigh Scale Height (m)", &atmosphereParameters.rayleighScaleHeight, 100.0f,
                    100.0f, 50000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Mie"))
            {
                transmittanceDirty |= ImGui::DragFloat(
                    "Mie Density Multiplier", &atmosphereParameters.mieDensityMultiplier, 0.05f,
                    0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                transmittanceDirty |= editCoefficient("Mie Scattering (1e-6 m^-1)",
                                                      atmosphereParameters.mieScattering);
                transmittanceDirty |= editCoefficient("Mie Absorption (1e-6 m^-1)",
                                                      atmosphereParameters.mieAbsorption);
                transmittanceDirty |=
                    ImGui::DragFloat("Mie Scale Height (m)", &atmosphereParameters.mieScaleHeight,
                                     50.0f, 100.0f, 10000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("Mie Asymmetry", &atmosphereParameters.mieAsymmetry, -0.99f,
                                   0.99f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Ozone"))
            {
                const float atmosphereHeight =
                    atmosphereParameters.AtmosphereRadius - atmosphereParameters.groundRadius;
                transmittanceDirty |= editCoefficient("Ozone Absorption (1e-6 m^-1)",
                                                      atmosphereParameters.ozoneAbsorption);
                transmittanceDirty |= ImGui::DragFloat(
                    "Ozone Center Height (m)", &atmosphereParameters.ozoneCenterHeight, 100.0f,
                    0.0f, atmosphereHeight, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                transmittanceDirty |= ImGui::DragFloat(
                    "Ozone Half Width (m)", &atmosphereParameters.ozoneHalfWidth, 100.0f, 100.0f,
                    atmosphereHeight, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Shadow Settings"))
        {
            ImGui::DragFloat("Shadow Bias (m)", &shadowBias, 0.5f, 0.0f, 100.0f, "%.1f",
                             ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragFloat("Shadow Slope Bias", &shadowSlopeBias, 0.05f, 0.0f, 4.0f, "%.2f",
                             ImGuiSliderFlags_AlwaysClamp);
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
            screenSkyShader->setBool("uEnableSkyView",
                                     skyViewEnabled && displayMode == DisplayMode::Scene);
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
            shader->setMat4("uLightViewProjection", lightViewProjection);
            shader->setFloat("uShadowBias", shadowBias / (sunFarPlane - sunNearPlane));
            shader->setVec3("uSunDirection", atmosphereParameters.sunDirection());
            shader->setVec3("uSunIlluminance", atmosphereParameters.sunIlluminance);
            shader->setFloat("uGroundRadius", atmosphereParameters.groundRadius);
            shader->setFloat("uTopRadius", atmosphereParameters.AtmosphereRadius);

            atmosphere->getTransmittanceLut().bind(0);
            sunFramebuffer.getDepthTexture().bind(6);

            for (const RenderObject &object : objects)
            {
                object.draw(camera);
            }
        }
    }

    void Application::renderSunView()
    {
        const glm::vec3 sunDirection = atmosphereParameters.sunDirection();
        if (sunViewValid && sunDirection == lastSunDirection
            && shadowSlopeBias == lastShadowSlopeBias)
        {
            return;
        }

        const glm::vec3 target(0.0f, 1500.0f, 0.0f);
        const glm::vec3 lightPosition = target + sunDirection * 60'000.0f;

        const glm::vec3 lightUp =
            glm::abs(glm::dot(sunDirection, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
            ? glm::vec3(0.0f, 0.0f, 1.0f)
            : glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 lightView = glm::lookAt(lightPosition, target, lightUp);
        const glm::mat4 lightProjection =
            glm::ortho(-36'000.0f, 36'000.0f, -36'000.0f, 36'000.0f, sunNearPlane, sunFarPlane);
        lightViewProjection = lightProjection * lightView;

        sunFramebuffer.bind();

        const Texture2D &texture = sunFramebuffer.getColorTexture();
        glViewport(0, 0, texture.getWidth(), texture.getHeight());
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sunViewShader->use();
        sunViewShader->setVec3("uSunDirection", sunDirection);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(shadowSlopeBias, 0.0f);

        for (const RenderObject &object : objects)
        {
            object.draw(*sunViewShader, lightView, lightProjection);
        }

        glDisable(GL_POLYGON_OFFSET_FILL);

        lastSunDirection = sunDirection;
        lastShadowSlopeBias = shadowSlopeBias;
        sunViewValid = true;
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

        if (displayMode == DisplayMode::SunView)
        {
            const int size = std::min(window.getWidth(), window.getHeight());
            glViewport((window.getWidth() - size) / 2, (window.getHeight() - size) / 2, size, size);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            glViewport(0, 0, window.getWidth(), window.getHeight());
        }
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_FRAMEBUFFER_SRGB);

        atmosphere->getTransmittanceLut().bind(0);
        atmosphere->getSkyViewLut().bind(1);
        atmosphere->getAerialPerspectiveLut().bind(2);
        sceneFramebuffer.getColorTexture().bind(3);
        sceneFramebuffer.getDepthTexture().bind(4);
        sunFramebuffer.getColorTexture().bind(5);
        sunFramebuffer.getDepthTexture().bind(6);

        screenMeshShader->use();
        atmosphere->setCommonUniforms(*screenMeshShader, atmosphereParameters);
        screenMeshShader->setMat4("uLightViewProjection", lightViewProjection);
        screenMeshShader->setFloat("uShadowBias", shadowBias / (sunFarPlane - sunNearPlane));
        screenMeshShader->setBool("uEnableAerialPerspective",
                                  aerialPerspectiveEnabled && displayMode == DisplayMode::Scene);
        screenMeshShader->setBool("uUseAerialPerspectiveLut", useAerialPerspectiveLut);
        screenMeshShader->setFloat("uAerialPerspectiveMaxDepth", camera.getFarPlane());
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
        screenMeshShader->setBool("uShowSunView", displayMode == DisplayMode::SunView);
        screenMeshShader->setBool("uShowAerialPerspectiveLut",
                                  displayMode == DisplayMode::AerialPerspectiveLut);
        screenMeshShader->setFloat("uAerialPerspectiveDebugDepth", aerialPerspectiveDebugDepth);
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
            instance->sunViewShader.reset();
            instance->finalShader.reset();
            instance->shader.reset();
            instance->sceneFramebuffer.release();
            instance->sunFramebuffer.release();
            instance->window.notifyClosed();
            instance->atmosphere.reset();
        }
    }
}
