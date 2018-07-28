#include "subsys.hpp"

namespace gfx
{
    std::vector<char> loadTextFile(const std::string &path)
    {
        std::fstream fs(path, std::fstream::in | std::fstream::ate);
        if (fs.is_open())
        {
            auto size = fs.tellg();
            fs.seekg(0, std::fstream::beg);
            
            std::vector<char> buffer(size);
            fs.read(buffer.data(), size);
            
            return buffer;
        }
        
        return std::vector<char>();
    }
    
    GLenum glBufferUsage(BufferUsage usage)
    {
        switch (usage) {
            case BufferUsage::StaticDraw: return GL_STATIC_DRAW;
            case BufferUsage::StreamDraw: return GL_STREAM_DRAW;
        };
    }

    GLenum glIndexType(IndexType type)
    {
        switch (type) {
            case IndexType::UByte: return GL_UNSIGNED_BYTE;
            case IndexType::UShort: return GL_UNSIGNED_SHORT;
            case IndexType::Uint: return GL_UNSIGNED_INT;
        };
    }
}

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
    
    GLenum gl4TexType(Texture::Type typ)
    {
        switch(typ)
        {
            case Texture::Type::Tex2D: return GL_TEXTURE_2D;
            case Texture::Type::TexRectangle: return GL_TEXTURE_RECTANGLE;
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
    Texture::Texture(const cv::Mat &source, Type t)
    : Texture(source.cols, source.rows, texFormat(source), true, source.data, t)
    {}
    
    Texture::Texture(int width, int height, Format format, bool smoothFilter, void *pixelData, Type t)
    : width_(width)
    , height_(height)
    , format_(format)
    , type_(t)
    {
        GLenum GL_TEX_TYPE = gl4TexType(type_);
        
        glGenTextures(1, &tex_);
        glBindTexture(GL_TEX_TYPE, tex_);
        if (smoothFilter)
        {
            glTexParameterf(GL_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameterf(GL_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glTexParameterf(GL_TEX_TYPE, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEX_TYPE, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEX_TYPE, 0, gl4Format(format), width, height, 0, gl4Format(format), gl4Type(format), pixelData);
    }
    
    Texture::Texture(Texture &&other)
    : tex_(other.tex_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
    , type_(other.type_)
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
            type_ = other.type_;
        }
        return *this;
    }
    
    void Texture::updateTexelData(cv::Mat &source)
    {
        assert(source.cols == width_);
        assert(source.rows == height_);
        assert(texFormat(source) == format_);
        
        GLenum GL_TEX_TYPE = gl4TexType(type_);
        
        glBindTexture(GL_TEX_TYPE, tex_);
        glTexSubImage2D(GL_TEX_TYPE, 0, 0, 0, width_, height_, gl4Format(format_), gl4Type(format_), source.data);
    }
    
    void Texture::updateTexelData(void *texelData)
    {
        GLenum GL_TEX_TYPE = gl4TexType(type_);
        
        glBindTexture(GL_TEX_TYPE, tex_);
        glTexSubImage2D(GL_TEX_TYPE, 0, 0, 0, width_, height_, gl4Format(format_), gl4Type(format_), texelData);
    }
    
    void Texture::bind()
    {
        GLenum GL_TEX_TYPE = gl4TexType(type_);
        glBindTexture(GL_TEX_TYPE, tex_);
    }
    
    
// Program
//
    bool Program::init(const std::string &shaderName)
    {
        auto isCompilationError = [](GLuint shader, const std::string &filepath)
        {
            GLint length, result;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
            if(result == GL_FALSE) {
                /* get the shader info log */
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                char *log = (char *)malloc(length);
                glGetShaderInfoLog(shader, length, &result, log);
                
                /* print an error message and the info log */
                fprintf(stderr, "Program::init(): Unable to compile %s: %s\n", filepath.c_str(), log);
                free(log);
                return true;
            }
            return false;
        };
        
        GLuint vs = 0, fs = 0;
        
        std::string ver = "#version 120\n";
        auto filename = shaderName + ".shader";
        auto src = loadTextFile(filename);
        
        
        // create vertex shader
        {
            const char *defVS = "#define VERTEX_SHADER 1\n#define FRAGMENT_SHADER 0\n";
            GLchar const* sources[] = { ver.c_str(), defVS, src.data() };
            GLint lengths[] = { (GLint)ver.size(), (GLint)strlen(defVS), (GLint)src.size() };

            vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 3, sources, lengths);
            glCompileShader(vs);
            if (isCompilationError(vs, filename + "/vs"))
            {
                glDeleteShader(vs);
                return false;
            }
        }

        // create fragment shader
        {
            const char *defFS = "#define VERTEX_SHADER 0\n#define FRAGMENT_SHADER 1\n";
            GLchar const* sources[] = { ver.c_str(), defFS, src.data() };
            GLint lengths[] = { (GLint)ver.size(), (GLint)strlen(defFS), (GLint)src.size() };

            fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 3, sources, lengths);
            glCompileShader(fs);
            if (isCompilationError(fs, filename + "/fs"))
            {
                glDeleteShader(fs);
                return false;
            }
        }
        
        // create program
        {
            prg = glCreateProgram();
            glAttachShader(prg, vs);
            glAttachShader(prg, fs);
            
            glDeleteShader(vs);
            glDeleteShader(fs);
        }
        
        // link program
        {
            glLinkProgram(prg);
            
            GLint length, result;
            glGetProgramiv(prg, GL_LINK_STATUS, &result);
            if(result == GL_FALSE) {
                /* get the program info log */
                glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &length);
                char *log = (char *)malloc(length);
                glGetProgramInfoLog(prg, length, &result, log);
                
                /* print an error message and the info log */
                fprintf(stderr, "Program::init(): Program linking failed: %s\n", log);
                free(log);
                
                /* delete the program */
                glDeleteProgram(prg);
                prg = 0;
                return false;
            }
        }
        
        return true;
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
    RGBFeed::RGBFeed()
    : pngWriteThread(&RGBFeed::writePNG, this)
    {
        freeBuffers_.reserve(BufferCount_);
        for (int k = 0; k < BufferCount_; ++k)
        {
            freeBuffers_.push_back(k);
        }
    }
    
    RGBFeed::~RGBFeed()
    {
        bThreadRunning_ = false;
        pngWriteThread.join();
    }
    
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
            for (int k = 0; k < BufferCount_; ++k)
            {
                imageTexBuf_[k].resize(texWidth * texHeight * 4);
            }
            
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

            initPNG(imd);
        }

        // Process this frame
        {
            std::lock_guard<std::mutex> freeBuffersGuard(freeBuffersMutex_);
            if (freeBuffers_.size() > 0)
            {
                // get a free buffer
                //
                int currentBuffer = freeBuffers_.back();
                freeBuffers_.pop_back();
                
                // Update texture data
                //
                {
                    const XnRGB24Pixel* pImageRow = imd.RGB24Data();
                    XnRGB24Pixel* pTexRow = (XnRGB24Pixel*)(imageTexBuf_[currentBuffer].data() + imd.YOffset() * texWidth);
                    
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

                    tex_.updateTexelData((void *)imageTexBuf_[currentBuffer].data());
                }
                
                // send this buffer to Write thread for writing to disk
                //
                std::lock_guard<std::mutex> buffersToWriteGuard(buffersToWriteMutex_);
                buffersToWrite_.push_back(currentBuffer);
            }
            else
            {
                //std::cout << "RGB frame skipped!\n";
            }
        }
    }
    
    void RGBFeed::initPNG(xn::ImageMetaData &imd)
    {
        imgW_ = imd.XRes();
        imgH_ = imd.YRes();
        
        for (int b = 0; b < BufferCount_; ++b)
        {
            rowPointers_[b].resize(imgH_);
            
            XnRGB24Pixel* pTexRow = (XnRGB24Pixel*)(imageTexBuf_[b].data() + imd.YOffset() * texWidth);
            for (auto k = 0; k < imgH_; ++k)
            {
                rowPointers_[b][k] = reinterpret_cast<png_bytep>(pTexRow);
                pTexRow += texWidth;
            }
        }
        
        currentFrame_ = 0;

        outdir_ = "./rgbfeed";
        if (!boost::filesystem::create_directory(outdir_)) {
            return;
        }
        /*{
             // Use rgbfeed/ as output directory
             //  If rgbfeed/ already exists then use rgbfeed1/ as output directory
             //  If rgbfeed1/ already exists then use rgbfeed2/ as output directory
             //
             //  If all 3 directories exists then overwrite rgbfeed/
         
             if (boost::filesystem::exists(outdir_))
             {
                 for (int k = 1; k < 3; ++k)
                 {
                     std::string newOutDir(outdir_);
                     newOutDir += std::to_string(k);
         
                     if (boost::filesystem::exists(newOutDir)) continue;
         
                     if (boost::filesystem::create_directory(newOutDir))
                     {
                        outdir_ = newOutDir;
                     }
                 }
             }
             else
             {
                if (!boost::filesystem::create_directory(outdir_)) return;
             }
         }*/
    }
    
    void RGBFeed::writePNG()
    {
        static bool bWriteToFile = true;
        
        // Enter write thread loop
        while(bThreadRunning_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            int currentBuffer = -1;
            {
                std::lock_guard<std::mutex> buffersToWriteGuard(buffersToWriteMutex_);
                if (buffersToWrite_.size() == 0) continue;
                
                currentBuffer = buffersToWrite_.front();
                buffersToWrite_.pop_front();
                
                if (bWriteToFile)
                {
                    png_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                    if (!png_) abort();
                    
                    pngInfo_ = png_create_info_struct(png_);
                    if (!pngInfo_) abort();
                    
                    if (setjmp(png_jmpbuf(png_))) abort();

                    std::string filename = outdir_ + "/frame" + std::to_string(currentFrame_++) + ".png";
                    
                    FILE *fp = fopen(filename.c_str(), "wb");
                    if(fp)
                    {
                        png_init_io(png_, fp);
                        
                        // Output is 8bit depth, RGB format.
                        png_set_IHDR(
                                     png_,
                                     pngInfo_,
                                     imgW_, imgH_,
                                     8,
                                     PNG_COLOR_TYPE_RGB,
                                     PNG_INTERLACE_NONE,
                                     PNG_COMPRESSION_TYPE_DEFAULT,
                                     PNG_FILTER_TYPE_DEFAULT
                                     );
                        png_write_info(png_, pngInfo_);
                    
                        png_write_image(png_, rowPointers_[currentBuffer].data());
                    }
                    
                    png_write_end(png_, NULL);
                    fclose(fp);
                }
            }
            
            if (currentBuffer != -1)
            {
                std::lock_guard<std::mutex> freeBuffersGuard(freeBuffersMutex_);
                freeBuffers_.push_back(currentBuffer);
            }
        }
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
        auto *tex = target->getTexture();
        return begin((GLuint)tex->platformHandle(), tex->width(), tex->height(), target->logicalWidth(), target->logicalHeight());
    }
    
    bool RenderToTexture::begin(Texture *target)
    {
        return begin((GLuint)target->platformHandle(), target->width(), target->height(), target->width(), target->height());
    }
    
    bool RenderToTexture::begin(GLuint target, int w, int h, int vw, int vh)
    {
        if (frameBuffer == 0) // first run
        {
            glGenFramebuffers(1, &frameBuffer);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

        if (depthRenderBuffer == 0 && bUseDepth)
        {
            glGenRenderbuffers(1, &depthRenderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
        }
        if (bUseDepth && (w != width || h != height))
        {
            width = w;
            height = h;
            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        }
        
        if (window::isLegacyOpenGL())
            glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, 0);
        else
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, 0);

        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
        
        glViewport(0, 0, vw, vh);
        
        return true;
    }
    
    void RenderToTexture::end()
    {
        if (window::isLegacyOpenGL())
            glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
        else
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    
// gfx::drawString()
//
#if defined(USE_FONT_SIZE_32)
    #include "helvetica-32.cxx"
#elif defined(USE_FONT_SIZE_12)
    #include "helvetica-12.cxx"
#endif
    namespace
    {
        struct TextRenderer
        {
            static bool bError;

            struct Vertex
            {
                float x, y;
                float u, v;
            };
            static const int CharactersMax = 32;
            static const int BufferVertexCountMax = CharactersMax * 4;
            static Vertex VertexData[BufferVertexCountMax];

            static const int BufferIndexCountMax = CharactersMax * 6;

            static bool bStatesSet;
            
            static std::unique_ptr<VertexBuffer> VB;
            static std::unique_ptr<IndexBuffer> IB;
            static std::unique_ptr<Texture> FontTexture;
            static std::unique_ptr<Program> ShaderProgram;
            
            static GLint texRectLoc;
            
        private:
            static Font    *font;
            static int     nCharactersToDraw;
            static int     x, y;

        public:
            static void setXY(int x, int y)
            {
                TextRenderer::x = x;
                TextRenderer::y = y;
            }
            
            static void drawChar(char c, float textScale=1.0f)
            {
                using f = float;
                
                if (c < 32 || c > 126) return;
                
#if defined(USE_FONT_SIZE_32)
                float scale = 0.35f * textScale; // scale of the font
#elif defined(USE_FONT_SIZE_12)
                float scale = 1.0f * textScale; // scale of the font
#endif
                
                auto *chrInfo = &font->characters[c - 32];
                int ox = 0;//-chrInfo->originX * scale;
                int oy = -chrInfo->originY * scale;
                int &u = chrInfo->x;
                int &v = chrInfo->y;
                int &w = chrInfo->width;
                int &h = chrInfo->height;
                
                int sw = w * scale;
                int sh = h * scale;
                
                Vertex *quad = &TextRenderer::VertexData[4 * nCharactersToDraw];
                quad[0] = {f(x + ox),       f(y + oy),         f(u),       f(v)};
                quad[1] = {f(x + sw + ox),  f(y + oy),         f(u + w),   f(v)};
                quad[2] = {f(x + sw + ox),  f(y + sh + oy),    f(u + w),   f(v + h)};
                quad[3] = {f(x + ox),       f(y + sh + oy),    f(u),       f(v + h)};
                
                //x += font->characters['W' - 32].width * scale;
                x += (c == ' ')? font->characters['W' - 32].width * 0.5f * scale : sw;
                
                if (++nCharactersToDraw == TextRenderer::CharactersMax)
                {
                    drawChars();
                }
            }
            
            static void flush()
            {
                if (nCharactersToDraw)
                {
                    drawChars();
                }
            }
            
            static bool setStates();
            
        private:
            static void drawChars()
            {
                if (!TextRenderer::bStatesSet)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    //glEnable(GL_ALPHA_TEST);
                    //glAlphaFunc(GL_GREATER, 0.4);
                    
                    TextRenderer::setStates();
                    TextRenderer::bStatesSet = true;
                }
                TextRenderer::VB->updateData(TextRenderer::VertexData, 4 * nCharactersToDraw);
                TextRenderer::IB->draw(6 * nCharactersToDraw);
                nCharactersToDraw = 0;
            }
        };
        bool TextRenderer::bError = false;
        TextRenderer::Vertex TextRenderer::VertexData[TextRenderer::BufferVertexCountMax];
        bool TextRenderer::bStatesSet = false;
        std::unique_ptr<VertexBuffer> TextRenderer::VB;
        std::unique_ptr<IndexBuffer> TextRenderer::IB;
        std::unique_ptr<Texture> TextRenderer::FontTexture;
        std::unique_ptr<Program> TextRenderer::ShaderProgram;
        GLint TextRenderer::texRectLoc;
        
        Font *TextRenderer::font = &font_Helvetica;
        int TextRenderer::nCharactersToDraw = 0;
        int TextRenderer::x;
        int TextRenderer::y;

        bool TextRenderer::setStates()
        {
            if (TextRenderer::bError) return false;
            
            if (!TextRenderer::FontTexture)
            {
                cv::Mat image;
#if defined(USE_FONT_SIZE_32)
                image = cv::imread("data/helvetica-32.png", CV_LOAD_IMAGE_UNCHANGED);   // Read the file
#elif defined(USE_FONT_SIZE_12)
                image = cv::imread("data/helvetica-12.png", CV_LOAD_IMAGE_UNCHANGED);   // Read the file
#endif
                if(image.data )
                {
                    TextRenderer::FontTexture = std::make_unique<Texture>(image, Texture::Type::TexRectangle);
                }
                else {
                    TextRenderer::bError = true;
                    return false;
                }
            }
            TextRenderer::FontTexture->bind();
            
            
            if (!TextRenderer::IB)
            {
                TextRenderer::IB = std::make_unique<IndexBuffer>();
                ushort data[TextRenderer::BufferIndexCountMax];
                for (int k = 0; k < TextRenderer::CharactersMax; ++k)
                {
                    ushort *face = &data[6 * k];
                    ushort i = 4 * k;
                    *face++ = i+0; *face++ = i+1; *face++ = i+2;
                    *face++ = i+0; *face++ = i+2; *face++ = i+3;
                }
                TextRenderer::IB->init<gfx::IndexType::UShort>(TextRenderer::BufferIndexCountMax, BufferUsage::StaticDraw, data);
            }
            TextRenderer::IB->bind();
            
            
            if (!TextRenderer::VB)
            {
                TextRenderer::VB = std::make_unique<VertexBuffer>();
                TextRenderer::VB->init<TextRenderer::Vertex>(TextRenderer::BufferVertexCountMax, BufferUsage::StreamDraw);
            }
            TextRenderer::VB->bind();
            
    #define BUFFER_OFFSET(i) ((void*)(i))
            
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, sizeof(TextRenderer::Vertex), BUFFER_OFFSET(0));    // The starting point of the VBO, for the vertices
            glClientActiveTexture(GL_TEXTURE0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(TextRenderer::Vertex), BUFFER_OFFSET(8)); // The starting point of texcoords, 8 bytes away
            
            
            if (!TextRenderer::ShaderProgram)
            {
                TextRenderer::ShaderProgram = std::make_unique<Program>();
                if (!TextRenderer::ShaderProgram->init("data/font"))
                {
                    TextRenderer::bError = true;
                    return false;
                }
                TextRenderer::ShaderProgram->bind();
                TextRenderer::texRectLoc = TextRenderer::ShaderProgram->uniformLocation("texRect");
            }
            TextRenderer::ShaderProgram->bind();
            glUniform1i (TextRenderer::texRectLoc, 0);
            
            return true;
        }
    }
    void drawString(int x, int y, const char *str, float scale)
    {
        if (!TextRenderer::bError)
        {
            TextRenderer::bStatesSet = false;
            TextRenderer::setXY(x, y);
            
            char *nextChar = const_cast<char *>(str);
            while (*nextChar)
            {
                TextRenderer::drawChar(*nextChar++, scale);
            }
            
            TextRenderer::flush();
            if (TextRenderer::bStatesSet)
            {
                glDisable(GL_BLEND);
                //glDisable(GL_ALPHA_TEST);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glUseProgram(0);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }
    }
    
} // namespace gfx


