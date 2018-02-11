#ifndef subsys_
#define subsys_

/* The classes below are exported */
#pragma GCC visibility push(default)

/* stdlib */
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <type_traits>
#include <cassert>


/* OpenNI */
#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <XnPropNames.h>


/* OpenCV */
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


/* GL */
#include <GL/glew.h>
#include <OpenGL/gl.h>


/* other */
#include "windowbase.hpp"
#include "utils.hpp"
#include "imgui_internal.h"
//#include "imgui_tabs.h"


namespace window
{
    void create(const char *name, DrawCallback drawFunc);
    
    bool updateAll();
    
    ImVec2 windowSize(const char *name);
}


namespace sensor
{
    enum class Message
    {
        NewUser,
        LostUser,
        CallibrationInProgress,
        PoseInProgess
    };
    using Callback = std::function<void(Message, XnUserID)>; //std::add_pointer<void (Message, XnUserID)>::type;
    
    void start(Callback func);
    
    void updateAll();
    
    xn::DepthGenerator &depthGenerator();
    xn::UserGenerator &userGenerator();
    xn::ImageGenerator &imageGenerator();
}


namespace gfx
{
// Texture
//
    class Texture
    {
    public:
        enum Format
        {
            Rgb8,
            Bgra8,
            Bgr8,
            L8
        };
        
        Texture()
        : tex_(0), width_(0), height_(0)
        {}
        Texture(const cv::Mat &source);
        Texture(int width, int height, Format format, bool smoothfilter = true, void *pixelData = nullptr);
        Texture(Texture &&);
        ~Texture();
        
        Texture &operator= (Texture &&);
        
        void updateTexelData(cv::Mat &source);
        void updateTexelData(void *texelData);
        
        uint32_t platformHandle() const { return tex_; }
        int width() const { return width_; }
        int height() const { return width_; }
        Format format() const { return format_; }
        
    private:
        Texture(Texture &) = delete;
        Texture &operator= (Texture &) = delete;
        
        GLuint tex_;
        int width_, height_;
        Format format_;
    };
    
    
// DepthVisualization
//
    class DepthVisualization
    {
    public:
        Texture *getTexture() { return &tex_; }
        
        void update();
        
    private:
        float topLeftX, topLeftY, bottomRightY, bottomRightX, texXpos, texYpos;
        std::vector<unsigned char> depthTexBuf_;
        Texture tex_;
        bool bInit = false;
        unsigned int texWidth, texHeight;
    };
    

// RGBFeed
//
    class RGBFeed
    {
    public:
        Texture *getTexture() { return &tex_; }
        
        void update();
        
    private:
        float topLeftX, topLeftY, bottomRightY, bottomRightX, texXpos, texYpos;
        std::vector<unsigned char> imageTexBuf_;
        Texture tex_;
        bool bInit = false;
        unsigned int texWidth, texHeight;
    };
    
    
// Functions
//
    void drawRect(Texture *);
}

#pragma GCC visibility pop
#endif
