#include "subsys.hpp"
#include "har.hpp"

// OpenGLHelper Implementation
//
void OpenGLHelper::init()
{
    glClearColor(0.5, 0.5, 0.5, 0.0);
}

void OpenGLHelper::beginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLHelper::endFrame()
{
}

#ifdef notused
GLfloat texcoords[8];
void OpenGLHelper::drawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
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

void OpenGLHelper::drawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    
    drawRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OpenGLHelper::glPrintString(void *font, char *str)
{
    size_t i,l = strlen(str);
    
    for(i=0; i<l; i++)
    {
        //glutBitmapCharacter(font,*str++);
    }
}

void  OpenGLHelper::drawJoint(xn::UserGenerator& userGenerator,
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
const char * OpenGLHelper::getJointName (XnSkeletonJoint eJoint)
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
void  OpenGLHelper::drawLimb(xn::UserGenerator& userGenerator,
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
void  OpenGLHelper::drawPoint(xn::UserGenerator& userGenerator,
                              xn::DepthGenerator& depthGenerator,
                              XnUserID player, XnSkeletonJoint eJoint,
                              ofstream &x_file, bool addComma)
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
void  OpenGLHelper::distance3D(xn::UserGenerator& userGenerator,
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
void  OpenGLHelper::showVelocity(xn::UserGenerator& userGenerator,
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

void  OpenGLHelper::drawSkeleton(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player)
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
#endif


