#ifndef __IMGUI_IMPL_GLFW_H__
#define __IMGUI_IMPL_GLFW_H__

#include "windowbase.hpp"

struct GLFWwindow;

struct Window
{
    GLFWwindow *win;
    window::DrawCallback drawCallback;
    ImGuiContext *imguictx;
};

namespace imguih
{
    bool init();
    void shutdown();
    void newWindow(GLFWwindow *window);
    void newFrame(GLFWwindow *window);
}

#endif // __IMGUI_IMPL_GLFW_H__