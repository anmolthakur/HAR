#ifndef har_h
#define har_h

#include "graph_generators.hpp"
#include "bargraph_generator.hpp"
#include "states_info.hpp"

// OpenGL helper
//
class OpenGLHelper
{
public:
    void init();
    void beginFrame();
    
#ifdef notused
    void drawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY);
    void drawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY);
    void glPrintString(void *font, char *str);
    
    void drawSkeleton(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player);
    void showVelocity(xn::UserGenerator& userGenerator,
                                     xn::DepthGenerator& depthGenerator,
                                     XnUserID player, XnSkeletonJoint eJoint);
    void distance3D(xn::UserGenerator& userGenerator,
                                   xn::DepthGenerator& depthGenerator,
                    XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
    void drawPoint(xn::UserGenerator& userGenerator,
                    xn::DepthGenerator& depthGenerator,
                    XnUserID player, XnSkeletonJoint eJoint,
                    ofstream &x_file, bool addComma=true);
    void drawLimb(xn::UserGenerator& userGenerator,
                    xn::DepthGenerator& depthGenerator,
                    XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
    const char *getJointName (XnSkeletonJoint eJoint);
    void drawJoint(xn::UserGenerator& userGenerator,
                   xn::DepthGenerator& depthGenerator,
                   XnUserID player, XnSkeletonJoint eJoint);
#endif

    void endFrame();
};


// GUI Helper
//
class GUIHelper
{
    ImVec2 windowSize;
    
// MainPanel
    enum MainPanelTab { RGB, DEPTH };
    MainPanelTab currentMainPanelTab = RGB;
    
// LeftPanel
    bool bLeftPanelOpen = true;
    float someproperty0 = 3.14f;
    float someproperty1 = 2.718f;
    float someproperty2 = -1;
    
// RightPanel
    bool bRightPanelOpen = true;
    enum RightPanelTab { HAND_TRAJECTORIES, PREDICTED_TRAJECTORIES, DISTANCE_GRAPH, ANGULAR_GRAPH };
    RightPanelTab currentRightPanelTab = HAND_TRAJECTORIES;
    
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
