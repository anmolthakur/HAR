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
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void OpenGLHelper::endFrame()
{
}

void OpenGLHelper::glPrintString(void *font, char *str)
{
    size_t i,l = strlen(str);
    
    for(i=0; i<l; i++)
    {
        //glutBitmapCharacter(font,*str++);
    }
}

void OpenGLHelper::drawSkeleton(bool isDepthView)
{
    if (!sensor::initialized()) return;
    
    glDisable(GL_TEXTURE_2D);
    
    if (isDepthView) drawSkeletoninDepthView();
    else drawSkeletoninRGBView();
    
    glEnable(GL_TEXTURE_2D);
}

void OpenGLHelper::DrawLimb(xn::UserGenerator& userGenerator,
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

void OpenGLHelper::DrawJoint(xn::UserGenerator& userGenerator,
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
    XnSkeletonJointPosition joint;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);
    if (joint.fConfidence < 0.5){
        return;}
    
    XnPoint3D pt[2];
    pt[0] = joint.position;
    depthGenerator.ConvertRealWorldToProjective(1, pt, pt);
    glVertex2f(pt[0].X, pt[0].Y);
}

const char *OpenGLHelper::GetJointName (XnSkeletonJoint eJoint)
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
        default: return "Joint";
    };
    return "Joint";
}

void OpenGLHelper::DrawPoint(xn::UserGenerator& userGenerator,
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
            x_file << "[,]" << (addComma? ", " : "");
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
        }
        
    }
}

void OpenGLHelper::Distance3D(xn::UserGenerator& userGenerator,
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
    sprintf(strLabel, " Distance between %s, and %s, :(%.03fm) ", GetJointName(eJoint1), GetJointName(eJoint2), dist);
    glColor3f(1.f,1.f,1.f);
    glRasterPos2i(20, (eJoint2 == XN_SKEL_RIGHT_HAND)? 80 : 110);
    glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
    
}

void OpenGLHelper::DrawCircle(xn::UserGenerator& userGenerator,
                xn::DepthGenerator& depthGenerator,
                XnUserID player, XnSkeletonJoint eJoint, float radius, XnFloat *color3f)
{
    
    XnSkeletonJointPosition joint;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);
    
    if (joint.fConfidence < 0.5){
        return;
    }
    
    XnPoint3D pt;
    pt = joint.position;
    depthGenerator.ConvertRealWorldToProjective(1, &pt, &pt);
    float cx = pt.X;
    float cy = pt.Y;
    float r = radius;
    int num_segments = 16;
    
    glColor3f(color3f[0], color3f[1], color3f[2]);
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx , cy);
    for(int i = 0; i <= num_segments; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);//get the current angle
        
        float x = r * cosf(theta);//calculate the x component
        float y = r * sinf(theta);//calculate the y component
        
        glVertex2f(x +cx , y + cy);//output vertex
        
    }
    glEnd();
}

void OpenGLHelper::drawSkeletoninDepthView()
{
    char strLabel[100];
    XnUserID aUsers[3];
    XnUInt16 nUsers = 3;
    GLfloat width = 3;
    
    auto &depthGenerator = sensor::depthGenerator();
    auto &userGenerator = sensor::userGenerator();
    
    userGenerator.GetUsers(aUsers, nUsers);
    for (int i = 0; i < nUsers; ++i)
    {
        // Update targets for hand history objects
        {
            XnSkeletonJointPosition headJoint;
            userGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_HEAD, headJoint);
            
            XnPoint3D pt_world, pt_screen;
            pt_world = headJoint.position;
            depthGenerator.ConvertRealWorldToProjective(1, &pt_world, &pt_screen);
            
            g_LeftHandPositionHistory.SetTarget(pt_world, pt_screen);
            g_RightHandPositionHistory.SetTarget(pt_world, pt_screen);
        }
        
        char filename[256];
        sprintf (filename, "JointPositionData:%02d.csv", i);
        ofstream csv_file;
        csv_file.open (filename, ios::app);
        
        if (g_bPrintID)
        {
            XnPoint3D com;
            userGenerator.GetCoM(aUsers[i], com);
            depthGenerator.ConvertRealWorldToProjective(1, &com, &com);// I need to change with image generator
            glVertex2f(com.X, com.Y);
            //float tmpCOM_x =com.X;
            //float tmpCOM_y =com.Y;
            
            // Calculate the distance from the qrcode to camera
            float dist = com.Z /1000.0f;
            sprintf(strLabel, "Distance :(%.1fm) ", dist);
            glColor3f(1.f,1.f,1.f);
            glRasterPos2i(20, 60);
            glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
            
            sprintf(strLabel, "Hand speeds (L/R) :(%.1f/%.1f) ", g_LeftHandPositionHistory.Speed(), g_RightHandPositionHistory.Speed());
            glColor3f(1.f,1.f,1.f);
            glRasterPos2i(20, 130);
            glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
            
            xnOSMemSet(strLabel, 0, sizeof(strLabel));
            if (!g_bPrintState)
            {
                // Tracking
                sprintf(strLabel, "%d", aUsers[i]);
            }
            else if (userGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
            {
                // Tracking
                sprintf(strLabel, "%d - Tracking", aUsers[i]);
                
                
            }
            else if (userGenerator.GetSkeletonCap().IsCalibrating(aUsers[i]))
            {
                // Calibrating
                sprintf(strLabel, "%d - Calibrating...", aUsers[i]);
            }
            else
            {
                // Nothing
                sprintf(strLabel, "%d - Looking for pose", aUsers[i]);
            }
            
            
            glColor4f(1-Colors[i%nColors][0], 1-Colors[i%nColors][1], 1-Colors[i%nColors][2], 1);
            
            glRasterPos2i(com.X, com.Y);
            glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
        }
        
        //cmdVelPub.publish(vel);
        
        if (g_bDrawSkeleton && userGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
        {
            glLineWidth(width);
            glBegin(GL_LINES);
            glColor4f(1-Colors[aUsers[i]%nColors][0], 1-Colors[aUsers[i]%nColors][1], 1-Colors[aUsers[i]%nColors][2], 1);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_HEAD, XN_SKEL_NECK);
            
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
            
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
            
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);
            
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);
            
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
            
            DrawLimb(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);
            glEnd();
            
            
            glBegin(GL_POINTS);
            glColor3f(1.f, 0.f, 0.f);
            glPointSize(1000.0);
            
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_HEAD);
            
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_NECK);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_SHOULDER);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_ELBOW);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_NECK);
            
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_SHOULDER);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_ELBOW);
            
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_TORSO);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HIP);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_KNEE);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_HIP);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_FOOT);
            
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_KNEE);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HIP);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_FOOT);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_HAND);
            DrawJoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HAND);
            
            glEnd();
            
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_HEAD, csv_file);
            
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_NECK, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_SHOULDER, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_ELBOW, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_NECK, csv_file);
            
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_SHOULDER, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_ELBOW, csv_file);
            
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_TORSO, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HIP, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_KNEE, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_HIP, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_FOOT, csv_file);
            
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_KNEE, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HIP, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_FOOT, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_HAND, csv_file);
            DrawPoint(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HAND, csv_file, false);
            
            Distance3D(userGenerator, depthGenerator, aUsers[i], XN_SKEL_HEAD, XN_SKEL_RIGHT_HAND);
            Distance3D(userGenerator, depthGenerator, aUsers[i], XN_SKEL_HEAD, XN_SKEL_LEFT_HAND);
            
            DrawCircle(userGenerator, depthGenerator, aUsers[i], XN_SKEL_RIGHT_HAND, 10, g_RightHandPositionHistory.Color());
            DrawCircle(userGenerator, depthGenerator, aUsers[i], XN_SKEL_LEFT_HAND, 10, g_LeftHandPositionHistory.Color());
        }
        
        csv_file.close();
    }
}

void OpenGLHelper::drawSkeletoninRGBView()
{
    
}

