#include "subsys.hpp"
#include "har.hpp"


//namespace
//{
    const char *gWindowName = "Human Activity Prediction";
    
    void *GLUT_BITMAP_HELVETICA_18 = nullptr;
    
    XnBool g_bDrawSkeleton = TRUE;
    XnBool g_bPrintID = TRUE;
    XnBool g_bPrintState = TRUE;
    XnBool g_bPrintFrameID = FALSE;
    
    History g_RightHandPositionHistory;
    History g_LeftHandPositionHistory;
//}


// Application logic class
//
class Application
{
    OpenGLHelper ogl;
    GUIHelper gui;
    gfx::DepthVisualization depthViz;
    gfx::RGBFeed rgbFeed;
    gfx::RenderToTexture rtt;
    
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
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            
            if (gui.getCurrentMainPanelTab() == GUIHelper::MainPanelTab::RGB)
            {
                rgbFeed.update();
                
                glOrtho(0, rgbFeed.logicalWidth(), rgbFeed.logicalHeight(), 0, -1.0, 1.0);
                
                if (rtt.begin(&rgbFeed))
                {
                    ogl.drawSkeleton(false);
                }
                rtt.end();
            }
            else if (gui.getCurrentMainPanelTab() == GUIHelper::MainPanelTab::DEPTH)
            {
                depthViz.update();
                
                glOrtho(0, depthViz.logicalWidth(), depthViz.logicalHeight(), 0, -1.0, 1.0);
                
                if (rtt.begin(&depthViz))
                {
                    ogl.drawSkeleton(true);
                }
                rtt.end();
            }
            
            glPopMatrix();
        }
        ogl.endFrame();
        break;
        
    case window::Layer::GUI:
        gui.beginFrame(window::windowSize(gWindowName));
        {
            gui.drawCurrentScreen(depthViz, rgbFeed);
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



