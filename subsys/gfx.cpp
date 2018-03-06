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
 //New function added
    
    GLfloat texcoords[8];
    void DrawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
    {
        GLfloat verts[8] = {    topLeftX, topLeftY,
            topLeftX, bottomRightY,
            bottomRightX, bottomRightY,
            bottomRightX, topLeftY
        };
        glVertexPointer(2, GL_FLOAT, 0, verts);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        //TODO: Maybe glFinish needed here instead - if there's some bad graphics crap
        glFlush();
    }
    void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
        
        DrawRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);
        
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
 
    //Another function added
    void glPrintString(void *font, char *str)
    {
        size_t i,l = strlen(str);
        
        for(i=0; i<l; i++)
        {
            glutBitmapCharacter(font,*str++);
        }
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
    
} // namespace gfx


//Again add another function

//Draw ecah joints
void DrawJoint(xn::UserGenerator& userGenerator,
               xn::DepthGenerator& depthGenerator,
               XnUserID player, XnSkeletonJoint eJoint)
{
    char strLabel[50] = "";
    xnOSMemSet(strLabel, 0, sizeof(strLabel));
    
    if (!userGenerator.GetSkeletonCap().IsTracking(player))
    {
        printf("not tracked!\n");
        return;
    }
    if (!userGenerator.GetSkeletonCap().IsJointActive(eJoint))
    {
        return;
    }
    
    XnSkeletonJointPosition joint;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);
    if (joint.fConfidence < 0.5){
        return;}
    
    XnPoint3D pt[2];
    pt[0] = joint.position;
    depthGenerator.ConvertRealWorldToProjective(1, pt, pt);
    glVertex2f(pt[0].X, pt[0].Y);
}

//Define all the joints name in each joints
const char *GetJointName (XnSkeletonJoint eJoint)
{
    
    switch (eJoint)
    {
        case XN_SKEL_HEAD: return "head";
        case XN_SKEL_NECK: return "neck";
        case XN_SKEL_LEFT_SHOULDER: return "left shoulder";
        case XN_SKEL_LEFT_ELBOW: return "left elbow";
        case XN_SKEL_RIGHT_SHOULDER: return "right shoulder";
        case XN_SKEL_RIGHT_ELBOW: return "right elbow";
        case XN_SKEL_TORSO: return "torse";
        case XN_SKEL_LEFT_HIP: return "left hip";
        case XN_SKEL_LEFT_KNEE: return "left knee";
        case XN_SKEL_LEFT_FOOT: return "left foot";
        case XN_SKEL_RIGHT_KNEE: return "right knee";
        case XN_SKEL_RIGHT_FOOT: return "right foot";
        case XN_SKEL_RIGHT_HAND: return "right hand";
        case XN_SKEL_LEFT_HAND: return "left hand";
            
    };
    
    return "Joint";
}

//Draw the line between two joints
void DrawLimb(xn::UserGenerator& userGenerator,
              xn::DepthGenerator& depthGenerator,
              XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
    if (!userGenerator.GetSkeletonCap().IsTracking(player))
    {
        printf("not tracked!\n");
        return;
    }
    
    XnSkeletonJointPosition joint1, joint2;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);
    
    if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5)
    {
        return;
    }
    
    XnPoint3D pt[2];
    pt[0] = joint1.position;
    pt[1] = joint2.position;
    
    depthGenerator.ConvertRealWorldToProjective(2, pt, pt);
    
    glVertex3i(pt[0].X, pt[0].Y, 0);
    glVertex3i(pt[1].X, pt[1].Y, 0);
}

//Draw each points in all the joints
void DrawPoint(xn::UserGenerator& userGenerator,
               xn::DepthGenerator& depthGenerator,
               XnUserID player, XnSkeletonJoint eJoint,
               ofstream &x_file, bool addComma=true)
{
    char strLabel[50] = "";
    xnOSMemSet(strLabel, 0, sizeof(strLabel));
    
    if (!userGenerator.GetSkeletonCap().IsTracking(player))
    {
        printf("not tracked!\n");
        return;
    }
    
    
    if (g_bPrintID){
        XnSkeletonJointPosition joint;
        userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);
        
        if (joint.fConfidence < 0.5){
            x_file << "[,]" << (addComma? ", " : "\r\n");
            return;
        }
        else {
            
            XnPoint3D pt[2];
            pt[0] = joint.position;
            depthGenerator.ConvertRealWorldToProjective(1, pt, pt);
            glVertex2f(pt[0].X, pt[0].Y);
            sprintf(strLabel, "%s (%.0f, %.0f) ", GetJointName(eJoint), pt[0].X,pt[0].Y);
            glColor3f(1.f,1.f,1.f);
            glRasterPos2i(pt[0].X, pt[0].Y);
            glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
            
            x_file << "[" << pt[0].X << ", " << pt[0].Y << "]";
            if (addComma) x_file << ", ";
            else x_file << "\r\n";
        }
        
    }
}

//Calculate 3D distance between two points inorder to evaluate threshold in prediction
void Distance3D(xn::UserGenerator& userGenerator,
                xn::DepthGenerator& depthGenerator,
                XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
    char strLabel[50] = "";
    xnOSMemSet(strLabel, 0, sizeof(strLabel));
    
    if (!userGenerator.GetSkeletonCap().IsTracking(player))
    {
        printf("not tracked!\n");
        return;
    }
    
    XnSkeletonJointPosition joint1, joint2;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);
    
    if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5)
    {
        return;
    }
    
    
    XnVector3D v;
    v.X = joint1.position.X - joint2.position.X;
    v.Y = joint1.position.Y - joint2.position.Y;
    v.Z = joint1.position.Z - joint2.position.Z;
    float distance3D = sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
    
    float dist = sqrt(distance3D * distance3D) * 0.001f;
    //sprintf(strLabel, " Distance between %s", GetJointName(eJoint1), "and %s", GetJointName(eJoint2),":(%.03fm) ", dist);
    //glColor3f(1.f,1.f,1.f);
    //glRasterPos2i(20, (eJoint2 == XN_SKEL_RIGHT_HAND)? 80 : 110);
    //glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
    
}


//Evaluate the velocity of both left and right hand
void Showvelocity(xn::UserGenerator& userGenerator,
                  xn::DepthGenerator& depthGenerator,
                  XnUserID player, XnSkeletonJoint eJoint)
{
    
    char strLabel[50] = "";
    xnOSMemSet(strLabel, 0, sizeof(strLabel));
    vector<double> joint_x;
    vector<double> joint_y;
    double jnt_x =0, jnt_y=0;
    
    if (!userGenerator.GetSkeletonCap().IsTracking(player))
    {
        printf("not tracked!\n");
        return;
    }
    
    
    XnSkeletonJointPosition joint;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);
    
    if (joint.fConfidence < 0.5){
        return;
    }
    
    XnPoint3D pt[2];
    pt[0] = joint.position;
    depthGenerator.ConvertRealWorldToProjective(1, pt, pt);
    glVertex2f(pt[0].X, pt[0].Y);
    
    double tmp_x =pt[0].X;
    double tmp_y =pt[0].Y;
    
    double posx_diff = tmp_x - jnt_x;
    double posy_diff = tmp_y - jnt_y;
    
    jnt_x = tmp_x;
    jnt_y = tmp_y;
    
    double dist = sqrt(posx_diff*posx_diff + posy_diff*posy_diff);
    
    double vel_x = posx_diff * fps;
    double vel_y = posy_diff * fps;
    double vel =  sqrt(vel_x*vel_x + vel_y*vel_y);
    double velocity = dist/fps;
    
    
    joint_x.push_back(vel_x);
    joint_y.push_back(vel_y);
    
}

void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player)
{
    static bool bInitialized = false;
    static GLuint depthTexID;
    static unsigned char* pDepthTexBuf;
    static int texWidth, texHeight;
    
    float topLeftX;
    float topLeftY;
    float bottomRightY;
    float bottomRightX;
    float texXpos;
    float texYpos;
    
    if(!bInitialized)
    {
        
        texWidth =  getClosestPowerOfTwo(dmd.XRes());
        texHeight = getClosestPowerOfTwo(dmd.YRes());
        
        //        printf("Initializing depth texture: width = %d, height = %d\n", texWidth, texHeight);
        depthTexID = initTexture((void**)&pDepthTexBuf,texWidth, texHeight) ;
        
        //        printf("Initialized depth texture: width = %d, height = %d\n", texWidth, texHeight);
        bInitialized = true;
        
        topLeftX = dmd.XRes();
        topLeftY = 0;
        bottomRightY = dmd.YRes();
        bottomRightX = 0;
        texXpos =(float)dmd.XRes()/texWidth;
        texYpos  =(float)dmd.YRes()/texHeight;
        
        memset(texcoords, 0, 8*sizeof(float));
        texcoords[0] = texXpos, texcoords[1] = texYpos, texcoords[2] = texXpos, texcoords[7] = texYpos;
        
    }
    unsigned int nValue = 0;
    unsigned int nHistValue = 0;
    unsigned int nIndex = 0;
    unsigned int nX = 0;
    unsigned int nY = 0;
    unsigned int nNumberOfPoints = 0;
    XnUInt16 g_nXRes = dmd.XRes();
    XnUInt16 g_nYRes = dmd.YRes();
    
    unsigned char* pDestImage = pDepthTexBuf;
    
    const XnDepthPixel* pDepth = dmd.Data();
    const XnLabel* pLabels = smd.Data();
    
    // Calculate the accumulative histogram
    memset(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
    for (nY=0; nY<g_nYRes; nY++)
    {
        for (nX=0; nX<g_nXRes; nX++)
        {
            nValue = *pDepth;
            
            if (nValue != 0)
            {
                g_pDepthHist[nValue]++;
                nNumberOfPoints++;
            }
            
            pDepth++;
        }
    }
    
    for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
    {
        g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
    }
    if (nNumberOfPoints)
    {
        for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
        {
            g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
        }
    }
    
    pDepth = dmd.Data();
    {
        XnUInt32 nIndex = 0;
        // Prepare the texture map
        for (nY=0; nY<g_nYRes; nY++)
        {
            for (nX=0; nX < g_nXRes; nX++, nIndex++)
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
                    nHistValue = g_pDepthHist[nValue];
                    
                    pDestImage[0] = nHistValue * Colors[nColorID][0];
                    pDestImage[1] = nHistValue * Colors[nColorID][1];
                    pDestImage[2] = nHistValue * Colors[nColorID][2];
                }
                else
                {
                    pDestImage[0] = 0;
                    pDestImage[1] = 0;
                    pDestImage[2] = 0;
                }
                
                pDepth++;
                pLabels++;
                pDestImage+=3;
            }
            
            pDestImage += (texWidth - g_nXRes) *3;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, depthTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pDepthTexBuf);
    
    // Display the OpenGL texture map
    glColor4f(0.75,0.75,0.75,1);
    
    glEnable(GL_TEXTURE_2D);
    DrawTexture(dmd.XRes(),dmd.YRes(),0,0);
    glDisable(GL_TEXTURE_2D);
    
    char strLabel[20] = "";
    XnUserID aUsers[15];
    XnUInt16 nUsers = 15;
    g_UserGenerator.GetUsers(aUsers, nUsers);
    for (int i = 0; i < nUsers; ++i)
    {
        XnPoint3D com;
        g_UserGenerator.GetCoM(aUsers[i], com);
        g_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);
        
        if (aUsers[i] == player)
            sprintf(strLabel, "%d (Player)", aUsers[i]);
        else
            sprintf(strLabel, "%d", aUsers[i]);
        
        glColor4f(1-Colors[i%nColors][0], 1-Colors[i%nColors][1], 1-Colors[i%nColors][2], 1);
        
        glRasterPos2i(com.X, com.Y);
        glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
    }
    
    // Draw skeleton of user
    if (player != 0)
    {
        glBegin(GL_LINES);
        glColor4f(1-Colors[player%nColors][0], 1-Colors[player%nColors][1], 1-Colors[player%nColors][2], 1);
        DrawLimb(player, XN_SKEL_HEAD, XN_SKEL_NECK);
        
        DrawLimb(player, XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
        DrawLimb(player, XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
        DrawLimb(player, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
        
        DrawLimb(player, XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
        DrawLimb(player, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
        DrawLimb(player, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
        
        DrawLimb(player, XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
        DrawLimb(player, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);
        
        DrawLimb(player, XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
        DrawLimb(player, XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
        DrawLimb(player, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);
        
        DrawLimb(player, XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
        DrawLimb(player, XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
        DrawLimb(player, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
        glEnd();
    }
}

