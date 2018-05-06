#include "subsys.hpp"
#include "har.hpp"

// GUIHelper Implementation
//
namespace
{
    template <typename EnumType>
    void doTabButton(const char *name, EnumType &variable, const EnumType value)
    {
        bool bPushed = false;
        if (variable == value)
        {
            bPushed = true;
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
        }
        if (ImGui::Button(name, ImVec2(90, 20)))
        {
            variable = value;
        }
        if (bPushed)
        {
            ImGui::PopStyleColor();
        }
    }
}

void GUIHelper::drawCurrentScreen(gfx::DynamicTextureGenerator &depthViz, gfx::DynamicTextureGenerator &rgbFeed)
{
    switch (currentScreen)
    {
    case Screen::Startup:
        {
            doStartupScreen();
        }
        break;
            
    case Screen::SingleTarget:
    case Screen::MultiTarget:
        {
            doMainContent(depthViz, rgbFeed);
            doLeftPanel();
            doRightPanel();
            doBottomPanel();
        }
        break;
    };
}

void GUIHelper::init()
{
}

void GUIHelper::beginFrame(const ImVec2 &windowSize)
{
    this->windowSize = windowSize;
}

void GUIHelper::endFrame()
{
}

void GUIHelper::doStartupScreen()
{
    const int windowWidth = 300;
    const int windowHeight = 60;

    auto screenWidth = this->windowSize.x;
    auto screenHeight = this->windowSize.y;

    ImGui::SetNextWindowPos(ImVec2((screenWidth - windowWidth) / 2, (screenHeight - windowHeight) / 2));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    ImGui::Begin("Human Activity Prediction Scenario", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    {
        ImGui::BeginColumns("button-cols", 2);
        {
            ImGui::SetColumnWidth(0, windowWidth / 2);
            if (ImGui::Button("Sequential Action"))
            {
                currentScreen = Screen::SingleTarget;
            }
            ImGui::NextColumn();
            
            ImGui::SetColumnWidth(1, windowWidth / 2);
            if (ImGui::Button("Concurrent Action"))
            {
                currentScreen = Screen::MultiTarget;
            }
            ImGui::NextColumn();
        }
        ImGui::EndColumns();
    }
    ImGui::End();
}

void GUIHelper::doMainContent(gfx::DynamicTextureGenerator &depthTexgen, gfx::DynamicTextureGenerator &rgbTexgen)
{
// Top BAR
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(windowSize.x, 30));
        ImGui::Begin("top-bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            if (ImGui::Button("<< Back", ImVec2(60, 20)))
            {
                currentScreen = Screen::Startup;
            }
            ImGui::SameLine();
            ImGui::Dummy(ImVec2((windowSize.x - 280)/2, 20));
            ImGui::SameLine();
            doTabButton("RGB", currentMainPanelTab, MainPanelTab::RGB);
            ImGui::SameLine();
            doTabButton("Depth", currentMainPanelTab, MainPanelTab::DEPTH);
        }
        ImGui::End();
    }
    
// Content (RGB/Depth)
    {
        auto wh = windowSize.y - 30;
        auto ww = wh * 4.0f / 3.0f;
        ImGui::SetNextWindowPos(ImVec2((windowSize.x - ww) / 2, 30));
        ImGui::SetNextWindowSize(ImVec2(ww, wh));
        ImGui::Begin("main-content", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs);
        {
            ImTextureID tex = 0;
            ImVec2 uv1, uv2;
            
            switch(currentMainPanelTab)
            {
            case MainPanelTab::RGB:
                {
                    tex = reinterpret_cast<void *>(rgbTexgen.getTexture()->platformHandle());
                    uv1 = rgbTexgen.getUV1();
                    uv2 = rgbTexgen.getUV2();
                }
                break;

            case MainPanelTab::DEPTH:
                {
                    tex = reinterpret_cast<void *>(depthTexgen.getTexture()->platformHandle());
                    uv1 = depthTexgen.getUV1();
                    uv2 = depthTexgen.getUV2();
                }
                break;
            };
            
            auto maxSize = ImGui::GetWindowContentRegionMax();
            auto minSize = ImGui::GetWindowContentRegionMin();
            auto iw = maxSize.x - minSize.x;
            auto ih = maxSize.y - minSize.y;
            ImGui::Image(tex, ImVec2(iw, ih), uv1, uv2);
        }
        ImGui::End();
    }
}

void GUIHelper::doLeftPanel()
{
    ImGui::SetNextWindowPos(ImVec2(0, 30));
    ImGui::SetNextWindowSize(ImVec2(200, windowSize.y - 230));
    ImGui::Begin("Properties", &bLeftPanelOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    {
        ImGui::LabelText("Prop1", "%0.3f", someproperty0);
        ImGui::LabelText("Prop2", "%0.3f", someproperty1);
        ImGui::LabelText("Prop3", "%0.3f", someproperty2);
    }
    ImGui::End();
}

void GUIHelper::doRightPanel()
{
    ImGui::SetNextWindowPos(ImVec2(windowSize.x - 400, 30));
    ImGui::SetNextWindowSize(ImVec2(400, windowSize.y - 230));
    ImGui::Begin("Graphs", &bRightPanelOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    {
        doTabButton("Hands", currentRightPanelTab, RightPanelTab::HAND_TRAJECTORIES); ImGui::SameLine();
        doTabButton("Predicted", currentRightPanelTab, RightPanelTab::PREDICTED_TRAJECTORIES); ImGui::SameLine();
        doTabButton("Distance", currentRightPanelTab, RightPanelTab::DISTANCE_GRAPH); ImGui::SameLine();
        doTabButton("Angle", currentRightPanelTab, RightPanelTab::ANGULAR_GRAPH);
        ImGui::Separator();
        
        switch(currentRightPanelTab)
        {
        case RightPanelTab::HAND_TRAJECTORIES:
                drawHandTrajectories();
            break;

        case RightPanelTab::PREDICTED_TRAJECTORIES:
                drawPredictedTrajectories();
            break;

        case RightPanelTab::DISTANCE_GRAPH:
                drawDistanceGraph();
            break;

        case RightPanelTab::ANGULAR_GRAPH:
                drawAngleGraph();
            break;
        };
    }
    ImGui::End();
}

void GUIHelper::doBottomPanel()
{
    static TestBarGraphGenerator graph;
    graph.update();
    
    static TestStatesInfo states;
    states.update();
    
    ImGui::SetNextWindowPos(ImVec2(0, windowSize.y - 200));
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, 200));
    ImGui::Begin("Plots", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    {
        using namespace ImGui;
        ImGuiWindow* wind = GetCurrentWindow();
        ImDrawList* dl = wind->DrawList;
        ImGuiDrawContext& dc = wind->DC;

        auto winMaxPt = GetWindowContentRegionMax();
        auto winMinPt = GetWindowContentRegionMin();
        
        ImVec2 pos = dc.CursorPos;
        auto w = winMaxPt.x - winMinPt.x;
        auto h = winMaxPt.y - winMinPt.y;
        
        auto wby2 = w / 2;
        auto plotWidth = wby2 - 10;
        
        auto bargraphMinPt = pos;
        auto bargraphMaxPt = ImVec2(pos.x + plotWidth, pos.y + h);
        dl->AddRectFilled(bargraphMinPt, bargraphMaxPt, 0x66000000, 10.0f);
        drawBarGraph(dl, bargraphMinPt, bargraphMaxPt, graph);
        
        auto statesMinPt = ImVec2(pos.x + wby2, pos.y);
        auto statesMaxPt = ImVec2(pos.x + wby2 + plotWidth, pos.y + h);
        dl->AddRectFilled(statesMinPt, statesMaxPt, 0x66000000, 10.0f);
        drawStates(dl, dc, statesMinPt, statesMaxPt, states);
    }
    ImGui::End();
}

void GUIHelper::drawHandTrajectories()
{
    static SinWaveGenerator sinWave(3, 20, -1.0f, 1.0f);
    static TriangleWaveGenerator triangleWave(3);
    static RandomWaveGenerator randomWave(3);
    
    using namespace ImGui;
    ImGuiWindow* wind = GetCurrentWindow();
    ImDrawList* dl = wind->DrawList;
    ImGuiDrawContext& dc = wind->DC;
    
    auto winMaxPt = GetWindowContentRegionMax();
    auto winMinPt = GetWindowContentRegionMin();
    
    ImVec2 pos = dc.CursorPos;
    auto w = winMaxPt.x - winMinPt.x;
    auto h = winMaxPt.y - winMinPt.y - 22;
    auto hby2 = h / 2;
    auto graphHeight = hby2 - 2;
    
    auto graphMinPt = pos;
    auto graphMaxPt = ImVec2(pos.x + w, pos.y + graphHeight);
    dl->AddRectFilled(graphMinPt, graphMaxPt, 0x66000000, 10.0f);
    {
        drawGraph(dl, graphMinPt, graphMaxPt, triangleWave, 0xffffffff);
    }
    
    graphMinPt = ImVec2(pos.x, pos.y + hby2);
    graphMaxPt = ImVec2(pos.x + w, pos.y + hby2 + graphHeight);
    dl->AddRectFilled(graphMinPt, graphMaxPt, 0x66000000, 10.0f);
    {
        drawGraph(dl, graphMinPt, graphMaxPt, sinWave, 0xffffffff);
    }
}

void GUIHelper::drawPredictedTrajectories()
{
    static SinWaveGenerator sinWave(3, 20, -1.0f, 1.0f);
    static TriangleWaveGenerator triangleWave(15);
    static SinWaveGenerator sinWave2(4, 25, -1.0f, 1.0f);
    static TriangleWaveGenerator triangleWave2(22);

    using namespace ImGui;
    ImGuiWindow* wind = GetCurrentWindow();
    ImDrawList* dl = wind->DrawList;
    ImGuiDrawContext& dc = wind->DC;
    
    auto winMaxPt = GetWindowContentRegionMax();
    auto winMinPt = GetWindowContentRegionMin();
    
    ImVec2 pos = dc.CursorPos;
    auto w = winMaxPt.x - winMinPt.x;
    auto h = winMaxPt.y - winMinPt.y - 22;
    auto hby2 = h / 2;
    auto graphHeight = hby2 - 2;
    
    auto graphMinPt = pos;
    auto graphMaxPt = ImVec2(pos.x + w, pos.y + graphHeight);
    dl->AddRectFilled(graphMinPt, graphMaxPt, 0x66000000, 10.0f);
    {
        drawGraph(dl, graphMinPt, graphMaxPt, sinWave, 0xff0000ff);
        drawGraph(dl, graphMinPt, graphMaxPt, triangleWave, 0xff00ff00);
    }
    
    if (currentScreen == Screen::SingleTarget)
    {
        graphMinPt = ImVec2(pos.x, pos.y + hby2);
        graphMaxPt = ImVec2(pos.x + w, pos.y + hby2 + graphHeight);
        dl->AddRectFilled(graphMinPt, graphMaxPt, 0x66000000, 10.0f);
        {
            drawGraph(dl, graphMinPt, graphMaxPt, triangleWave2, 0xff0000ff);
            drawGraph(dl, graphMinPt, graphMaxPt, sinWave2, 0xff00ff00);
        }
    }
    else if (currentScreen == Screen::MultiTarget)
    {
        graphMinPt = ImVec2(pos.x, pos.y + hby2);
        graphMaxPt = ImVec2(pos.x + w / 2, pos.y + hby2 + graphHeight);
        dl->AddRectFilled(graphMinPt, graphMaxPt, 0x440000ff, 10.0f);
        {
            drawGraph(dl, graphMinPt, graphMaxPt, triangleWave2, 0xff00ffff);
            drawGraph(dl, graphMinPt, graphMaxPt, sinWave2, 0xffff0000);
        }
        dl->AddText(graphMinPt, 0xffffffff, "X");

        graphMinPt = ImVec2(pos.x + w / 2, pos.y + hby2);
        graphMaxPt = ImVec2(pos.x + w, pos.y + hby2 + graphHeight);
        dl->AddRectFilled(graphMinPt, graphMaxPt, 0x4400ff00, 10.0f);
        {
            drawGraph(dl, graphMinPt, graphMaxPt, triangleWave2, 0xff00ffff);
            drawGraph(dl, graphMinPt, graphMaxPt, sinWave2, 0xffff0000);
        }
        dl->AddText(graphMinPt, 0xffffffff, "Y");
    }
}

void GUIHelper::drawDistanceGraph()
{
    static RandomWaveGenerator randomWave(30);
    
    using namespace ImGui;
    ImGuiWindow* wind = GetCurrentWindow();
    ImDrawList* dl = wind->DrawList;
    ImGuiDrawContext& dc = wind->DC;
    
    auto winMaxPt = GetWindowContentRegionMax();
    auto winMinPt = GetWindowContentRegionMin();
    
    ImVec2 pos = dc.CursorPos;
    auto w = winMaxPt.x - winMinPt.x;
    auto h = winMaxPt.y - winMinPt.y - 22;
    
    auto graphMinPt = pos;
    auto graphMaxPt = ImVec2(pos.x + w, pos.y + h);
    dl->AddRectFilled(graphMinPt, graphMaxPt, 0x66000000, 10.0f);
    {
        drawGraph(dl, graphMinPt, graphMaxPt, randomWave, 0xffffffff);
    }
}

void GUIHelper::drawAngleGraph()
{
    static SinWaveGenerator sinWave(3, 20, 0.0f, 1.0f);
    
    using namespace ImGui;
    ImGuiWindow* wind = GetCurrentWindow();
    ImDrawList* dl = wind->DrawList;
    ImGuiDrawContext& dc = wind->DC;
    
    auto winMaxPt = GetWindowContentRegionMax();
    auto winMinPt = GetWindowContentRegionMin();
    
    ImVec2 pos = dc.CursorPos;
    auto w = winMaxPt.x - winMinPt.x;
    auto h = winMaxPt.y - winMinPt.y - 22;
    
    auto graphMinPt = pos;
    auto graphMaxPt = ImVec2(pos.x + w, pos.y + h);
    dl->AddRectFilled(graphMinPt, graphMaxPt, 0x66000000, 10.0f);
    {
        drawGraph(dl, graphMinPt, graphMaxPt, sinWave, 0xffffffff, -1.0f, 1.0f);
    }
}

void GUIHelper::drawBarGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, BarGraphGenerator &graph)
{
    float width = maxPt.x - minPt.x;
    int N = graph.numBars();
    auto barWidth = width / N;

    float maxBarHeight = maxPt.y - minPt.y;
    
    ImGui::BeginColumns("col", 6);
    for (int k = 0; k < N; ++k)
    {
        float value = graph.valueAtBar(k) * 0.01f;
        
        ImVec2 bl(minPt.x + k * barWidth, maxPt.y); // bottom-left
        ImVec2 tr(minPt.x + k * barWidth + barWidth, maxPt.y - maxBarHeight * value); // top-right
        
        dl->AddRectFilled(bl, tr, 0xff0000ff);
        
        ImGui::SetColumnWidth(k, barWidth); ImGui::Text("Action"); ImGui::NextColumn(); //text need to change
    }
    ImGui::EndColumns();
}

void GUIHelper::drawStates(ImDrawList* dl, ImGuiDrawContext& dc, ImVec2 minPt, ImVec2 maxPt, StatesInfo &states)
{
    const int numStates = states.numStates();
    const int statesPerRow = 5;
    {
        const int numCols = statesPerRow;
        const int numRows = numStates / numCols + (numStates % numCols? 1 : 0);
        
        const float margin = 2;
        const float margin2 = margin * 2;
        
        const float width = maxPt.x - minPt.x;
        const float height = maxPt.y - minPt.y;
        const float rowHeight = height / numRows;
        const float colWidth = width / numCols;
        
        int currentState = 0;
        
        for (int r = 0; r < numRows; ++r)
        {
            ImVec2 rowStart(minPt.x, minPt.y + r * rowHeight);
            
            for (int c = 0; c < numCols; ++c)
            {
                ImVec2 colStart(rowStart.x + c * colWidth + margin, rowStart.y + margin);
                ImVec2 colEnd(rowStart.x + c * colWidth + colWidth - margin2, rowStart.y + rowHeight - margin2);
                
                if (states.isStateActive(currentState))
                {
                    dl->AddRectFilled(colStart, colEnd, 0xff666666);
                    dl->AddRect(colStart, colEnd, 0xffffffff);
                }
                else
                {
                    dl->AddRect(colStart, colEnd, 0xff666666);
                }
                
                ImGui::SetCursorScreenPos(colStart);
                ImGui::Text("State"); // text need to change
                
                ++currentState;
                if (currentState == numStates) break;
            }
            
            if (currentState == numStates) break;
        }
    }
}

void GUIHelper::drawGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, GraphGenerator &graph, ImU32 col)
{
    drawGraph(dl, minPt, maxPt, graph, col, graph.minValue(), graph.maxValue());
}

void GUIHelper::drawGraph(ImDrawList* dl, ImVec2 minPt, ImVec2 maxPt, GraphGenerator &graph, ImU32 col, float minValue, float maxValue)
{
    ImVec2 startPt(minPt.x, 0.5f * (minPt.y + maxPt.y));
    ImVec2 endPt(maxPt.x, 0.5f * (minPt.y + maxPt.y));
    
    dl->AddLine(startPt, endPt, 0x66ffffff, 1.0f);
    
    auto steps = graph.numSamples();
    assert(steps > 1 && "Steps shoud be greater than 1!");
    float deltaX = (maxPt.x - minPt.x) / (steps - 1);
    
    auto evaluatePt = [&](int sample)
    {
        float minY = maxPt.y;
        float maxY = minPt.y;
        
        float x = startPt.x + sample * deltaX;
        float y = (graph.valueAt(sample) - minValue) / (maxValue - minValue);
        y = y * maxY + (1.0f - y) * minY;
        
        return ImVec2(x, y);
    };
    
    ImVec2 prevPt = evaluatePt(0);
    
    for (auto i = 1; i < steps; ++i)
    {
        ImVec2 currPt = evaluatePt(i);
        
        dl->AddLine(prevPt, currPt, col, 1.0f);
        
        prevPt = currPt;
    }
}





