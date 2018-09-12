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
#include <boost/filesystem.hpp>
#include <thread>
#include <list>
#include <chrono>


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


//namespace
//{
    const static XnFloat Colors[][3] =
    {
        {0,1,1},
        {0,0,1}, // blue
        {0,1,0},
        {1,1,0}, // yellow
        {1,0,0}, // red
        {1,.5,0},
        {.5,1,0},
        {0,.5,1},
        {.5,0,1},
        {1,1,.5},
        {0.5,0.5,0.5},    // Grey
        {1,0,1}, // Purple
        {1,1,1}
    };
    const static XnUInt32 nColors = 12;
//}


namespace window
{
    using CreationFlag = uint;
    static const CreationFlag UseLegacyOpenGL = 0x1;
    
    void create(const char *name, DrawCallback drawFunc, CreationFlag flags = 0);
    
    bool updateAll();
    
    ImVec2 windowSize(const char *name);
    
    double getTime();
    bool isLegacyOpenGL();
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
    
    bool initialized();
    
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
        
        enum Type
        {
            Tex2D,
            TexRectangle
        };
        
        Texture()
        : tex_(0), width_(0), height_(0)
        {}
        Texture(const cv::Mat &source, Type t=Type::Tex2D);
        Texture(int width, int height, Format format, bool smoothfilter = true, void *pixelData = nullptr, Type t=Type::Tex2D);
        Texture(Texture &&);
        ~Texture();
        
        Texture &operator= (Texture &&);
        
        void updateTexelData(cv::Mat &source);
        void updateTexelData(void *texelData);
        
        uint32_t platformHandle() const { return tex_; }
        int width() const { return width_; }
        int height() const { return height_; }
        Format format() const { return format_; }
        Type type() const { return type_; }
        
        void bind();
        
    private:
        Texture(Texture &) = delete;
        Texture &operator= (Texture &) = delete;
        
        GLuint tex_;
        int width_, height_;
        Format format_;
        Type type_;
    };
    
    
// BufferUsage
//
    enum class BufferUsage
    {
        StaticDraw,
        StreamDraw
    };
    GLenum glBufferUsage(BufferUsage);

    
// Vertex buffer
//
    class VertexBuffer
    {
        using Usage = BufferUsage;

    public:
        VertexBuffer()
        {
            glGenBuffers(1, &vbo_);
        }
        ~VertexBuffer()
        {
            glDeleteBuffers(1, &vbo_);
        }
        
        uint32_t platformHandle() const { return vbo_; }
        Usage usage() const { return usage_; }
        uint32_t vertexCount() const { return vertexCount_; }
        size_t vertexSize() const { return vertexSize_; }
        
        template <typename Vertex>
        bool init(uint32_t vertexCount, Usage usage = Usage::StaticDraw, Vertex *data = nullptr)
        {
            usage_ = usage;
            vertexCount_ = vertexCount;
            vertexSize_ = sizeof(Vertex);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferData(GL_ARRAY_BUFFER, vertexSize_ * vertexCount_, data, glBufferUsage(usage_));
            return true;
        }
        
        template <typename Vertex>
        bool updateData(const Vertex *data, int count)
        {
            assert(vertexSize_ == sizeof(Vertex) && "");
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize_ * count, data);
            return true;
        }
        
        template <typename Vertex>
        Vertex *writeOnlyMap()
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            return (Vertex *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        }
        
        void unmap()
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
        
        void bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }
        
    private:
        GLuint vbo_ = 0;
        Usage usage_;
        uint32_t vertexCount_;
        size_t vertexSize_;
    };

    
// Index buffer
//
    enum IndexType { UByte, UShort, Uint };
    
    template <IndexType T> struct IndexPOD;
    template <> struct IndexPOD<IndexType::UByte> { using Type = unsigned char; };
    template <> struct IndexPOD<IndexType::UShort> { using Type = unsigned short; };
    template <> struct IndexPOD<IndexType::Uint> { using Type = uint32_t; };

    GLenum glIndexType(IndexType);

    class IndexBuffer
    {
        using Usage = BufferUsage;

    public:        
        IndexBuffer()
        {
            glGenBuffers(1, &ibo_);
        }
        ~IndexBuffer()
        {
            glDeleteBuffers(1, &ibo_);
        }
        
        uint32_t platformHandle() const { return ibo_; }
        Usage usage() const { return usage_; }
        uint32_t indexCount() const { return indexCount_; }
        
        template <IndexType T>
        bool init(uint32_t indexCount, Usage usage = Usage::StaticDraw, typename IndexPOD<T>::Type *data = nullptr)
        {
            usage_ = usage;
            indexCount_ = indexCount;
            indexDataType_ = T;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount_ * sizeof(typename IndexPOD<T>::Type), data, glBufferUsage(usage_));
            return true;
        }
        
        template <IndexType T>
        bool updateData(const typename IndexPOD<T>::Type *data, int count)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(typename IndexPOD<T>::Type), data);
            return true;
        }

        template <IndexType T>
        typename IndexPOD<T>::Type *writeOnlyMap()
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
            return (typename IndexPOD<T>::Type *) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        }
        
        void unmap()
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        }
        
        void bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_); }
        
        void draw(int count)
        {
            bind();
            glDrawElements(GL_TRIANGLES, count, glIndexType(indexDataType_), (void*)0);
        }
        
    private:
        GLuint ibo_ = 0;
        Usage usage_;
        uint32_t indexCount_;
        IndexType indexDataType_;
    };
    
    
// Program
//
    class Program
    {
    public:
        Program()
        {}
        ~Program()
        {
            if (prg)
            {
                glDeleteProgram(prg);
            }
        }
        
        uint32_t platformHandle() const { return prg; }
        
        bool init(const std::string &shaderName);
        
        void bind() { glUseProgram(prg); }
        
        int uniformLocation(const std::string &name) { return glGetUniformLocation(prg, name.c_str()); }
        
    private:
        GLuint prg = 0;
    };
    
    
    
// Dynamic Texture generator
//
    class DynamicTextureGenerator
    {
    public:
        virtual void update() = 0;
        virtual int logicalWidth() = 0;
        virtual int logicalHeight() = 0;
        
        Texture *getTexture() { return &tex_; }
        ImVec2 getUV1() { return uv1; }
        ImVec2 getUV2() { return uv2; }
        
    protected:
        Texture tex_;
        ImVec2 uv1, uv2;
    };
    
    
// DepthVisualization
//
    class DepthVisualization : public DynamicTextureGenerator
    {
    public:
        void update() override;
        int logicalWidth() override { return (int)topLeftX; }
        int logicalHeight() override { return (int)bottomRightY; }
        
    private:
        float topLeftX, topLeftY, bottomRightY, bottomRightX, texXpos, texYpos;
        std::vector<unsigned char> depthTexBuf_;
        bool bInit = false;
        unsigned int texWidth, texHeight;
    };
    

// RGBFeed
//
    class RGBFeed : public DynamicTextureGenerator
    {
    public:
        RGBFeed();
        ~RGBFeed();
        
        void update() override;
        int logicalWidth() override { return (int)topLeftX; }
        int logicalHeight() override { return (int)bottomRightY; }
        
    public:
        void captureFramebuffer();
        
    private:
        float topLeftX, topLeftY, bottomRightY, bottomRightX, texXpos, texYpos;
        std::vector<unsigned char> imageTexBuf_;
        bool bInit = false;
        unsigned int texWidth, texHeight;
        
        std::string outdir_, outdir2_;

        int imgW_, imgH_;
        int currentFrame_ = 0;
        int jpegQualitySetting = 50; // 95

        cv::VideoWriter video_pre;
        cv::VideoWriter video_post;
    };
    
    
// RenderToTexture
//
    class RenderToTexture
    {
        GLuint frameBuffer = 0;
        GLuint depthRenderBuffer = 0;
        int width = 0, height = 0;
        const bool bUseDepth;
        
    public:
        RenderToTexture(bool useDepth = false) : bUseDepth(useDepth)
        {}
        ~RenderToTexture();
        
        bool begin(DynamicTextureGenerator *target);
        bool begin(Texture *target);
        void end();
        
    private:
        bool begin(GLuint target, int w, int h, int vw, int vh);
    };
    
    
// Functions
//
    void drawString(int x, int y, const char *str, float scale=1.0f);
}

#pragma GCC visibility pop
#endif
