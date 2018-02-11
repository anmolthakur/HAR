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
    namespace {
        XnFloat Colors[][3] =
        {
            {0,1,1},
            {0,0,1},
            {0,1,0},
            {1,1,0},
            {1,0,0},
            {1,.5,0},
            {.5,1,0},
            {0,.5,1},
            {.5,0,1},
            {1,1,.5},
            {1,1,1}
        };
        XnUInt32 nColors = 10;
    }
    
    void DepthVisualization::update()
    {
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
        
            //memset(texcoords, 0, 8*sizeof(float));
            //texcoords[0] = texXpos, texcoords[1] = texYpos, texcoords[2] = texXpos, texcoords[7] = texYpos;
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
    
} // namespace gfx



