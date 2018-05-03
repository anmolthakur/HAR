#include "subsys.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_glfw.h"

namespace
{
    GLFWwindow *gFirstWindow = nullptr; // HACK: TODO.. Implement this properly. What is first windows was closed?
    
    GLFWwindow *createGLFWwindow(const char *name, void *user_ptr = nullptr)
    {
        fprintf(stdout, "GLFW: %s", glfwGetVersionString());
        
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        
        GLFWwindow *win;
        if (!(win = glfwCreateWindow(mode->width, mode->height, name, nullptr, gFirstWindow)))
        {
            fprintf(stderr, "GLFW error: glfwCreateWindow() failed.");
            ::exit(-1);
        }
        
        if (!gFirstWindow)
        {
            gFirstWindow = win;
            
            glfwMakeContextCurrent(win);
            
            if (GLenum err = glewInit(); err != GLEW_OK)
            {
                fprintf(stderr, "GLEW error: %s", glewGetErrorString(err));
                exit(-1);
            }
            
            fprintf(stdout, "Renderer: %s\n", glGetString(GL_RENDERER));
            fprintf(stdout, "OpenGL version supported %s\n", glGetString(GL_VERSION));
            fprintf(stdout, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
        }
        
        glfwSwapInterval(1);
        glfwSetWindowUserPointer(win, user_ptr);
        
        return win;
    }
}


// Window class
//
struct WindowImpl : public Window
{
    WindowImpl(const Window &) = delete;
    
    WindowImpl(const char *name, window::DrawCallback drawFunc)
    : Window{createGLFWwindow(name, this), drawFunc, ImGui::CreateContext()}
    {
        ImGui::SetCurrentContext(imguictx);
        imguih::newWindow(win);
    }
    
    ~WindowImpl ()
    {
        if (imguictx)
        {
            //ImGui::DestroyContext(imguictx); // TODO... find out why is it crashing at shutdown
        }
        if (win)
        {
            glfwDestroyWindow(win);
        }
    }
    
    static std::map<std::string, WindowImpl> allWindows;
    
public:
    // GLFW error callback
    static void errorCallback(int error, const char* description)
    {
        fprintf(stderr, "GLFW error: %s\n", description);
    }
    
    // Init GLFW and GLEW
    static bool init()
    {
        glfwSetErrorCallback(errorCallback);
        
        if (!glfwInit())
        {
            fprintf(stderr, "GLFW error: glfwInit() failed.");
            return false;
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        
        return imguih::init();
    }
    
    // shutdown GLFW
    static void shutdown()
    {
        imguih::shutdown();
        glfwTerminate();
    }
    
public:
    bool shouldClose()
    {
        return glfwWindowShouldClose(win);
    }
    
    void update()
    {
        // Draw scene
        drawCallback(window::Layer::BG);
        
        // Draw GUI
        ImGui::SetCurrentContext(imguictx);
        imguih::newFrame(win);
        drawCallback(window::Layer::GUI);
        ImGui::Render();
        
        // flip buffers
        glfwSwapBuffers(win);
    }
};


// Map of all windows created
//
std::map<std::string, WindowImpl> WindowImpl::allWindows;


// Global helper
//
struct WindosSysHelper
{
    WindosSysHelper ()
    {
        if (!WindowImpl::init())
        {
            exit(-1);
        }
    }
    
    ~WindosSysHelper ()
    {
        WindowImpl::allWindows.clear();
        WindowImpl::shutdown();
    }
} gwsh;



namespace window
{
    void create(const char *name, DrawCallback drawFunc)
    {
        if (WindowImpl::allWindows.size() == 0) // TODO... remove this limit of only 1 window
            // In order to remove this limit, implement context sharing
            // of GLFW windows properly and also properly implement
            // ImGui states.
        {
            WindowImpl::allWindows.emplace(std::piecewise_construct,
                                           std::forward_as_tuple(name),
                                           std::forward_as_tuple(name, drawFunc)
                                           );
        }
    }
    
    
    bool updateAll()
    {
        static std::vector<std::string> windowsToDestroy;
        windowsToDestroy.clear();
        
        glfwPollEvents();
        
        for (auto const& [name, win] : WindowImpl::allWindows)
        {
            auto &cwin = const_cast<WindowImpl &>(win);
            
            if (!cwin.shouldClose())
            {
                cwin.update();
            }
            else
            {
                windowsToDestroy.push_back(name);
            }
        }
        
        for(auto name : windowsToDestroy)
        {
            WindowImpl::allWindows.erase(name);
        }
        
        return WindowImpl::allWindows.size() > 0;
    }
    
    ImVec2 windowSize(const char *name)
    {
        auto &winImpl = WindowImpl::allWindows.at(name);
        int w, h;
        glfwGetWindowSize(winImpl.win, &w, &h);
        return ImVec2(w, h);
    }
    
    double getTime()
    {
        return glfwGetTime();
    }
}



