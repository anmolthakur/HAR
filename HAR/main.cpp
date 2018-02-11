#include "subsys.hpp"
#include "har.hpp"

// Application logic class
//
class Application
{
    OpenGLHelper ogl;
    GUIHelper gui;
    gfx::DepthVisualization depthViz;
    
private:
    void drawFunction(window::Layer layer);
    void sensorFunction(sensor::Message mssg, XnUserID id);

public:
    Application();
    
    int run();
};


// Program entry point
//
int main(int argc, char *argv[])
{
    Application app;
    return app.run();
}


// Application class Implementation
//
Application::Application()
{
    window::create("har", [this](window::Layer layer) {
        drawFunction(layer);
    });
    
    sensor::start([this](sensor::Message mssg, XnUserID id) {
        sensorFunction(mssg, id);
    });

    ogl.init();
    gui.init();
}

int Application::run()
{
    do
    {
        ::sleep(30);
        sensor::updateAll();
    }
    while(window::updateAll());
    
    return 0;
}

void Application::drawFunction(window::Layer layer)
{
    switch (layer)
    {
    case window::Layer::BG:
        ogl.beginFrame();
        {
            depthViz.update();
        }
        ogl.endFrame();
        break;
        
    case window::Layer::GUI:
        gui.beginFrame(window::windowSize("har"));
        {
            gui.doMainContent(depthViz.getTexture());
            gui.doLeftPanel();
            gui.doRightPanel();
            gui.doBottomPanel();
        }
        gui.endFrame();
        break;
    };
}

void Application::sensorFunction(sensor::Message mssg, XnUserID id)
{
    switch(mssg)
    {
    case sensor::Message::NewUser:
        break;
        
    case sensor::Message::LostUser:
        break;
        
    default:
        // TODO...
        break;
    };
}



