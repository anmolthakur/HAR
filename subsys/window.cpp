#include "subsys.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "windowbase.hpp"
//#include "imgui/imgui_impl_glfw.h"
//#include "imgui/imgui_impl_glfw_gl2.h"

namespace
{
    bool g_bLegacyOpengl = false;
    GLFWwindow *gFirstWindow = nullptr; // HACK: TODO.. Implement this properly. What is first windows was closed?
    
    GLFWwindow *createGLFWwindow(const char *name, bool bLegacyOpengl, void *user_ptr = nullptr)
    {
        fprintf(stdout, "GLFW: %s", glfwGetVersionString());
        
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        
        // TODO:
        //      Currently GL3 and GL2 cannot be mixed. All windows should have the same OpenGL.
        //      Update API to handle different versions opf OGL in different windows if possible.
        //
        if (bLegacyOpengl)
        {
            //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        }
        else
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
        //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        g_bLegacyOpengl = bLegacyOpengl;

        
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
struct WindowController
{
private:
    Window *pWin = nullptr;
    
public:
    WindowController(const WindowController &) = delete;
    
    WindowController(const char *name, window::DrawCallback drawFunc, bool bLegacyOpenGL)
    {
        pWin = bLegacyOpenGL? Window::createImGuiWindow_GL2() : Window::createImGuiWindow_GL3();
        pWin->init();
        pWin->win = createGLFWwindow(name, bLegacyOpenGL, this);
        pWin->drawCallback = drawFunc;
        pWin->imguictx = ImGui::CreateContext();
        
        ImGui::SetCurrentContext(pWin->imguictx);
        pWin->newWindow(pWin->win);
    }
    
    ~WindowController ()
    {
        if (pWin)
        {
            if (pWin->win)
            {
                glfwDestroyWindow(pWin->win);
            }
            if (pWin->imguictx)
            {
                //ImGui::DestroyContext(imguictx); // TODO... find out why is it crashing at shutdown
            }
            pWin->shutdown();
            delete pWin;
        }
    }
    
public:
    bool shouldClose()
    {
        return glfwWindowShouldClose(pWin->win);
    }
    
    void update()
    {
        // Draw scene
        pWin->drawCallback(window::Layer::BG);
        
        // Draw GUI
        ImGui::SetCurrentContext(pWin->imguictx);
        pWin->newFrame(pWin->win);
        pWin->drawCallback(window::Layer::GUI);
        ImGui::Render();
        
        // flip buffers
        glfwSwapBuffers(pWin->win);
    }
    
    GLFWwindow *getGlfwWindow() { return pWin->win; }
};


// Global helper
//
struct WindowSysHelper
{
    static std::map<std::string, WindowController> allWindows;

    static void errorCallback(int error, const char* description)
    {
        fprintf(stderr, "GLFW error: %s\n", description);
    }

    WindowSysHelper ()
    {
        glfwSetErrorCallback(errorCallback);
        
        if (!glfwInit())
        {
            fprintf(stderr, "GLFW error: glfwInit() failed.");
            exit(-1);
        }
    }
    
    ~WindowSysHelper ()
    {
        WindowSysHelper::allWindows.clear();
        glfwTerminate();
    }
} gwsh;


// Map of all windows created
//
std::map<std::string, WindowController> WindowSysHelper::allWindows;




namespace window
{
    void create(const char *name, DrawCallback drawFunc, CreationFlag flags)
    {
        if (WindowSysHelper::allWindows.size() == 0) // TODO... remove this limit of only 1 window
            // In order to remove this limit, implement context sharing
            // of GLFW windows properly and also properly implement
            // ImGui states.
        {
            WindowSysHelper::allWindows.emplace(std::piecewise_construct,
                                           std::forward_as_tuple(name),
                                                std::forward_as_tuple(name, drawFunc, flags & UseLegacyOpenGL? true : false)
                                           );
        }
    }
    
    
    bool updateAll()
    {
        static std::vector<std::string> windowsToDestroy;
        windowsToDestroy.clear();
        
        glfwPollEvents();
        
        for (auto const& [name, win] : WindowSysHelper::allWindows)
        {
            auto &cwin = const_cast<WindowController &>(win);
            
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
            WindowSysHelper::allWindows.erase(name);
        }
        
        return WindowSysHelper::allWindows.size() > 0;
    }
    
    ImVec2 windowSize(const char *name)
    {
        auto &winCtrlr = WindowSysHelper::allWindows.at(name);
        int w, h;
        glfwGetWindowSize(winCtrlr.getGlfwWindow(), &w, &h);
        return ImVec2(w, h);
    }
    
    double getTime()
    {
        return glfwGetTime();
    }
    
    bool isLegacyOpenGL()
    {
        return g_bLegacyOpengl;
    }
}



