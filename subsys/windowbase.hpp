#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <type_traits>
#include "imgui/imgui.h"

namespace window
{
    enum class Layer
    {
        BG,
        GUI
    };
    
    using DrawCallback = std::function<void(Layer)>; //std::add_pointer<void (Layer)>::type;
}

struct GLFWwindow;

struct Window
{
    GLFWwindow *win;
    window::DrawCallback drawCallback;
    ImGuiContext *imguictx;
    
    virtual ~Window()
    {}

    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual void newWindow(GLFWwindow *window) = 0;
    virtual void newFrame(GLFWwindow *window) = 0;

    static Window *createImGuiWindow_GL3();
    static Window *createImGuiWindow_GL2();
};

#endif //__WINDOW_HPP__

