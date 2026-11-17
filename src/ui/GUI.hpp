#pragma once

namespace atmogl
{
    class GUI
    {
    public:
        GUI();
        ~GUI();

        GUI(const GUI &) = delete;
        GUI &operator=(const GUI &) = delete;

        void beginFrame();
        void render();

        bool wantsMouse() const;
        bool wantsKeyboard() const;
    };
}