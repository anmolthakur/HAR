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
    void init();
    void beginFrame(const ImVec2 &windowSize);
    void endFrame();
    
    void doMainContent(gfx::Texture *depthTexture);
    void doLeftPanel();
    void doRightPanel();
    void doBottomPanel();
    
private:
    void drawHandTrajectories();
    void drawPredictedTrajectories();
    void drawDistanceGraph();
    void drawAngleGraph();
    
    void drawBarGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, BarGraphGenerator &graph);
    void drawStates(ImDrawList* dl, ImGuiDrawContext& dc, ImVec2 minPt, ImVec2 maxPt, StatesInfo &states);
    
    void drawGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, GraphGenerator &graph, ImU32 col);
    void drawGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, GraphGenerator &graph, ImU32 col, float minValue, float maxValue);
};

#endif /* har_h */
