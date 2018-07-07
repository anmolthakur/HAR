#ifndef har_h
#define har_h

#include <fstream>
using namespace std;

#include "graph_generators.hpp"
#include "bargraph_generator.hpp"
#include "states_info.hpp"
#include "trajectory.hpp"


//namespace
//{
    extern void *GLUT_BITMAP_HELVETICA_18;
    
    extern XnBool g_bDrawSkeleton;
    extern XnBool g_bPrintID;
    extern XnBool g_bPrintState;
    extern XnBool g_bPrintFrameID;
    
    extern History g_RightHandPositionHistory;
    extern History g_LeftHandPositionHistory;
//}


// OpenGL helper
//
class OpenGLHelper
{
public:
    void init();
    void beginFrame();

    void glPrintString(void *font, char *str, float scale=1.0f);
    void drawSkeleton(bool isDepthView);

    void endFrame();
    
private:
    void drawSkeletonCommon();
    void drawSkeletonInRGBView();
    void drawSkeletonInDepthView();
    
    void DrawLimb(xn::UserGenerator& userGenerator,
                  xn::DepthGenerator& depthGenerator,
                  XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
    
    void DrawJoint(xn::UserGenerator& userGenerator,
                   xn::DepthGenerator& depthGenerator,
                   XnUserID player, XnSkeletonJoint eJoint);
    
    const char *GetJointName (XnSkeletonJoint eJoint);
    
    enum DrawPointOptions {
        DRAW_NAME = 0x2,
        DRAW_POSITION = 0x4
    };
    void DrawPoint(xn::UserGenerator& userGenerator,
                   xn::DepthGenerator& depthGenerator,
                   XnUserID player, XnSkeletonJoint eJoint, uint drawPointOptions,
                   ofstream *x_file=nullptr, bool addComma=true);
    
    void Distance3D(xn::UserGenerator& userGenerator,
                    xn::DepthGenerator& depthGenerator,
                    XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
    
    void DrawCircle(xn::UserGenerator& userGenerator,
                    xn::DepthGenerator& depthGenerator,
                    XnUserID player, XnSkeletonJoint eJoint,
                    float radius, XnFloat *color3f);
    
    void DrawBezierCurve(const std::vector<XnPoint3D> &controlPoints, int numPoints = 16);
    
    void handtrajectory(xn::UserGenerator& userGenerator,
                        xn::DepthGenerator& depthGenerator,
                        XnUserID player, XnSkeletonJoint eJoint, bool updateHistory);
};


// GUI Helper
//
class GUIHelper
{
public:
    enum MainPanelTab { RGB, DEPTH };

private:
    ImVec2 windowSize;
    
// MainPanel
    MainPanelTab currentMainPanelTab = RGB;
    
// LeftPanel
    bool  bLeftPanelOpen = true;
    float someproperty0 = 3.14f;
    float someproperty1 = 2.718f;
    float someproperty2 = -1;
    
// RightPanel
    bool bRightPanelOpen = true;
    enum RightPanelTab { HAND_TRAJECTORIES, PREDICTED_TRAJECTORIES, DISTANCE_GRAPH, ANGULAR_GRAPH };
    RightPanelTab currentRightPanelTab = HAND_TRAJECTORIES;
    
public:
    MainPanelTab getCurrentMainPanelTab() { return currentMainPanelTab; }
    
public:
// Screens
    enum Screen { Startup, SingleTarget, MultiTarget };
    Screen currentScreen = Screen::Startup;
    
    void init();
    void beginFrame(const ImVec2 &windowSize);
    void drawCurrentScreen(gfx::DynamicTextureGenerator &depthTexgen, gfx::DynamicTextureGenerator &rgbTexgen);
    void endFrame();

private:
    void doStartupScreen();
    
    void doMainContent(gfx::DynamicTextureGenerator &depthTexgen, gfx::DynamicTextureGenerator &rgbTexgen);
    void doLeftPanel();
    void doRightPanel();
    void doBottomPanel();
    
private:
    void drawHandTrajectories();
    void drawPredictedTrajectories();
    void drawMultiplePredictedTrajectories();
    void drawDistanceGraph();
    void drawAngleGraph();
    
    void drawBarGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, BarGraphGenerator &graph);
    void drawStates(ImDrawList* dl, ImGuiDrawContext& dc, ImVec2 minPt, ImVec2 maxPt, StatesInfo &states);
    
    void drawGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, GraphGenerator &graph, ImU32 col);
    void drawGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, GraphGenerator &graph, ImU32 col, float minValue, float maxValue);
};

#endif /* har_h */
