#include "subsys.hpp"
#include "har.hpp"


namespace
{
    const char *gWindowName = "Human Activity Prediction";
}


// Application logic class
//
class Application
{
    OpenGLHelper ogl;
    GUIHelper gui;
    gfx::DepthVisualization depthViz;
    gfx::RGBFeed rgbFeed;
    
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
    window::create(gWindowName, [this](window::Layer layer) {
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
            rgbFeed.update();
        }
        ogl.endFrame();
        break;
        
    case window::Layer::GUI:
        gui.beginFrame(window::windowSize(gWindowName));
        {
            gui.doMainContent(depthViz, rgbFeed);
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



