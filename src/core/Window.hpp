#pragma once

#include <string>

namespace atmogl
{
    class Window
    {
    public:
        Window(int &argc, char **argv, int width, int height, const std::string &title);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        void startMainLoop() const;
        void swapBuffers() const;

        int getWidth() const;
        int getHeight() const;
        void setSize(int width, int height);
        void notifyClosed();

    private:
        int id = 0;
        int width;
        int height;
    };
}
