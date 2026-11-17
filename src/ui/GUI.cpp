#include "ui/GUI.hpp"

#include <GL/freeglut.h>
#include <imgui.h>
#include <imgui_impl_glut.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>

namespace atmogl
{
    GUI::GUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();

        if (!ImGui_ImplGLUT_Init())
        {
            ImGui::DestroyContext();
            throw std::runtime_error("Failed to initialize ImGui GLUT backend");
        }

        if (!ImGui_ImplOpenGL3_Init("#version 430 core"))
        {
            ImGui_ImplGLUT_Shutdown();
            ImGui::DestroyContext();
            throw std::runtime_error("Failed to initialize ImGui OpenGL backend");
        }

        ImGui_ImplGLUT_ReshapeFunc(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    }

    GUI::~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLUT_Shutdown();
        ImGui::DestroyContext();
    }

    void GUI::beginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGLUT_NewFrame();
        ImGui::NewFrame();
    }

    void GUI::render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool GUI::wantsMouse() const
    {
        return ImGui::GetIO().WantCaptureMouse;
    }

    bool GUI::wantsKeyboard() const
    {
        return ImGui::GetIO().WantCaptureKeyboard;
    }
}
