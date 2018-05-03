#include "subsys.hpp"

namespace
{
    using namespace gfx;
    
    GLenum gl4Format(Texture::Format fmt)
    {
        switch (fmt)
        {
            case Texture::Format::Rgb8:
                return GL_RGB;

            case Texture::Format::Bgra8:
                return GL_RGBA;//GL_BGRA;
                
            case Texture::Format::Bgr8:
                return GL_RGB;//GL_BGR;
                
            case Texture::Format::L8:
                return GL_LUMINANCE;
                
            default:
                assert(false && "Unsupported texture format!");
        };
        return 0;
    }
    
    GLenum gl4Type(Texture::Format fmt)
    {
        switch (fmt)
        {
            case Texture::Format::Rgb8:
            case Texture::Format::Bgra8:
            case Texture::Format::Bgr8:
            case Texture::Format::L8:
                return GL_UNSIGNED_BYTE;
                
            default:
                assert(false && "Unsupported texture format!");
        };
        return 0;
    }
    
    Texture::Format texFormat(const cv::Mat &image)
    {
        switch(image.channels())
        {
            case 4:
                return Texture::Format::Bgra8;
                
            case 3:
                return Texture::Format::Bgr8;
                
            case 1:
                return Texture::Format::L8;
                
            default:
                assert(false && "Unsupported cv::Image format!");
        }
    }
}

namespace gfx
{
// Texture
//
    Texture::Texture(const cv::Mat &source)
    : Texture(source.cols, source.rows, texFormat(source), true, source.data)
    {}
    
    Texture::Texture(int width, int height, Format format, bool smoothFilter, void *pixelData)
    : width_(width)
    , height_(height)
    , format_(format)
    {
        glGenTextures(1, &tex_);
        glBindTexture(GL_TEXTURE_2D, tex_);
        if (smoothFilter)
        {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, gl4Format(format), width, height, 0, gl4Format(format), gl4Type(format), pixelData);
    }
    
    Texture::Texture(Texture &&other)
    : tex_(other.tex_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
    {
        other.tex_ = 0;
    }
    
    Texture::~Texture()
    {
        if (tex_)
        {
            glDeleteTextures(1, &tex_);
            tex_ = 0;
        }
    }
    
    Texture &Texture::operator= (Texture &&other)
    {
        if (this != &other)
        {
            if (tex_)
            {
                glDeleteTextures(1, &tex_);
            }
            tex_ = other.tex_; other.tex_ = 0;
            
            width_ = other.width_;
            height_ = other.height_;
            format_ = other.format_;
        }
        return *this;
    }
    
    void Texture::updateTexelData(cv::Mat &source)
    {
        assert(source.cols == width_);
        assert(source.rows == height_);
        assert(texFormat(source) == format_);
        
        glBindTexture(GL_TEXTURE_2D, tex_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, gl4Format(format_), gl4Type(format_), source.data);
    }
    
    void Texture::updateTexelData(void *texelData)
    {
        glBindTexture(GL_TEXTURE_2D, tex_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, gl4Format(format_), gl4Type(format_), texelData);
    }
    
    
// DepthVisualization
//
    void DepthVisualization::update()
    {
        if (!sensor::initialized()) return;
        
        xn::SceneMetaData smd;
        xn::DepthMetaData dmd;
        sensor::depthGenerator().GetMetaData(dmd);
        sensor::userGenerator().GetUserPixels(0, smd);

        if (!bInit)
        {
            bInit = true;
    
            texWidth = getClosestPowerOfTwo(dmd.XRes());
            texHeight = getClosestPowerOfTwo(dmd.YRes());
            depthTexBuf_.resize(texWidth * texHeight * 4);

            tex_ = gfx::Texture(texWidth, texHeight, Texture::Format::Rgb8);
            
            topLeftX = dmd.XRes();
            topLeftY = 0;
            bottomRightY = dmd.YRes();
            bottomRightX = 0;
            texXpos = (float)dmd.XRes() / texWidth;
            texYpos = (float)dmd.YRes() / texHeight;
        
            uv1 = ImVec2(0.0f, 0.0f);
            uv2 = ImVec2(texXpos, texYpos);
            //memset(texcoords, 0, 8 * sizeof(float));
            //texcoords[0] = texXpos; texcoords[1] = texYpos; texcoords[2] = texXpos; texcoords[7] = texYpos;
        }
        
        // Update texture data
        {
            unsigned int nValue = 0;
            unsigned int nHistValue = 0;
            unsigned int nIndex = 0;
            unsigned int nX = 0;
            unsigned int nY = 0;
            unsigned int nNumberOfPoints = 0;
            XnUInt16 g_nXRes = dmd.XRes();
            XnUInt16 g_nYRes = dmd.YRes();
            
            unsigned char* pDestImage = depthTexBuf_.data();
            
            const XnDepthPixel* pDepth = dmd.Data();
            const XnLabel* pLabels = smd.Data();
            
            static unsigned int nZRes = dmd.ZRes();
            static float* pDepthHist = (float*)malloc(nZRes* sizeof(float));
            
            // Calculate the accumulative histogram
            memset(pDepthHist, 0, nZRes*sizeof(float));
            for (nY=0; nY<g_nYRes; nY++)
            {
                for (nX=0; nX<g_nXRes; nX++)
                {
                    nValue = *pDepth;
                    
                    if (nValue != 0)
                    {
                        pDepthHist[nValue]++;
                        nNumberOfPoints++;
                    }
                    
                    pDepth++;
                }
            }
            
            for (nIndex=1; nIndex<nZRes; nIndex++)
            {
                pDepthHist[nIndex] += pDepthHist[nIndex-1];
            }
            if (nNumberOfPoints)
            {
                for (nIndex=1; nIndex<nZRes; nIndex++)
                {
                    pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (pDepthHist[nIndex] / nNumberOfPoints)));
                }
            }
            
            pDepth = dmd.Data();
            //const bool g_bDrawPixels = true;
            const bool g_bDrawBackground = true;
            //if (g_bDrawPixels)
            {
                XnUInt32 nIndex = 0;
                // Prepare the texture map
                for (nY=0; nY<g_nYRes; nY++)
                {
                    for (nX=0; nX < g_nXRes; nX++, nIndex++)
                    {
                        
                        pDestImage[0] = 0;
                        pDestImage[1] = 0;
                        pDestImage[2] = 0;
                        if (g_bDrawBackground || *pLabels != 0)
                        {
                            nValue = *pDepth;
                            XnLabel label = *pLabels;
                            XnUInt32 nColorID = label % nColors;
                            if (label == 0)
                            {
                                nColorID = nColors;
                            }
                            
                            if (nValue != 0)
                            {
                                nHistValue = pDepthHist[nValue];
                                
                                pDestImage[0] = nHistValue * Colors[nColorID][0];
                                pDestImage[1] = nHistValue * Colors[nColorID][1];
                                pDestImage[2] = nHistValue * Colors[nColorID][2];
                            }
                        }
                        
                        pDepth++;
                        pLabels++;
                        pDestImage+=3;
                    }
                    
                    pDestImage += (texWidth - g_nXRes) *3;
                }
            }
            /*else
            {
                xnOSMemSet(pDepthTexBuf, 0, 3*2*g_nXRes*g_nYRes);
            }*/
            
            // TODO...
        }
        tex_.updateTexelData((void *)depthTexBuf_.data());
    }
   
    
// RGBFeed
//
    void RGBFeed::update()
    {
        if (!sensor::initialized()) return;
        
        xn::ImageMetaData imd;
        sensor::imageGenerator().GetMetaData(imd);
        
        if (!bInit)
        {
            bInit = true;
            
            texWidth = getClosestPowerOfTwo(imd.XRes());
            texHeight = getClosestPowerOfTwo(imd.YRes());
            imageTexBuf_.resize(texWidth * texHeight * 4);
            
            tex_ = gfx::Texture(texWidth, texHeight, Texture::Format::Rgb8);
            
            topLeftX = imd.XRes();
            topLeftY = 0;
            bottomRightY = imd.YRes();
            bottomRightX = 0;
            texXpos = (float)imd.XRes() / texWidth;
            texYpos = (float)imd.YRes() / texHeight;
            
            uv1 = ImVec2(0.0f, 0.0f);
            uv2 = ImVec2(texXpos, texYpos);
            //memset(texcoords, 0, 8 * sizeof(float));
            //texcoords[0] = texXpos; texcoords[1] = texYpos; texcoords[2] = texXpos; texcoords[7] = texYpos;
        }
        
        // Update texture data
        {
            const XnRGB24Pixel* pImageRow = imd.RGB24Data();
            XnRGB24Pixel* pTexRow = (XnRGB24Pixel*)(imageTexBuf_.data() + imd.YOffset() * texWidth);
            
            for (XnUInt y = 0; y < imd.YRes(); ++y)
            {
                const XnRGB24Pixel* pImage = pImageRow;
                XnRGB24Pixel* pTex = pTexRow + imd.XOffset();
                
                for (XnUInt x = 0; x < imd.XRes(); ++x, ++pImage, ++pTex)
                {
                    *pTex = *pImage;
                }
                
                pImageRow += imd.XRes();
                pTexRow += texWidth;
            }
        }
        tex_.updateTexelData((void *)imageTexBuf_.data());
    }
    
    
// RenderToTexture
//
    RenderToTexture::~RenderToTexture()
    {
        glDeleteRenderbuffers(1, &depthRenderBuffer);
        glDeleteFramebuffers(1, &frameBuffer);
    }
    
    bool RenderToTexture::begin(DynamicTextureGenerator *target)
    {
        return begin((GLuint)target->getTexture()->platformHandle(), target->logicalWidth(), target->logicalHeight());
    }
    
    bool RenderToTexture::begin(Texture *target)
    {
        return begin((GLuint)target->platformHandle(), target->width(), target->height());
    }
    
    bool RenderToTexture::begin(GLuint target, int w, int h)
    {
        if (frameBuffer == 0) // first run
        {
            glGenFramebuffers(1, &frameBuffer);
            glGenRenderbuffers(1, &depthRenderBuffer);
            
            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
            
            GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        
        if (w != width || h != height)
        {
            width = w;
            height = h;
            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        }
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
        
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, width, height);
        
        return true;
    }
    
    void RenderToTexture::end()
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
} // namespace gfx


