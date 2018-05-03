#include "subsys.hpp"

namespace
{
    bool g_Initialized = false;
    
    xn::Context g_Context;
    xn::ScriptNode g_scriptNode;
    xn::DepthGenerator g_DepthGenerator;
    xn::UserGenerator g_UserGenerator;
    xn::ImageGenerator g_ImageGenerator;
    xn::Player g_Player;
    
    XnBool g_bNeedPose = FALSE;
    XnChar g_strPose[20] = "";
    
    std::map<XnUInt32, std::pair<XnCalibrationStatus, XnPoseDetectionStatus> > m_Errors;
    
    sensor::Callback gCallbackFunc = nullptr;
}


namespace
{
    // Callback: New user was detected
    void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
    {
        XnUInt32 epochTime = 0;
        xnOSGetEpochTime(&epochTime);
        printf("%d New User %d\n", epochTime, nId);
        // New user found
        if (g_bNeedPose)
        {
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        }
        else
        {
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
    // Callback: An existing user was lost
    void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
    {
        XnUInt32 epochTime = 0;
        xnOSGetEpochTime(&epochTime);
        printf("%d Lost user %d\n", epochTime, nId);
        
        if (gCallbackFunc) gCallbackFunc(sensor::Message::LostUser, nId);
    }
    // Callback: Detected a pose
    void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* /*pCookie*/)
    {
        XnUInt32 epochTime = 0;
        xnOSGetEpochTime(&epochTime);
        printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
        g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
    // Callback: Started calibration
    void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/, XnUserID nId, void* /*pCookie*/)
    {
        XnUInt32 epochTime = 0;
        xnOSGetEpochTime(&epochTime);
        printf("%d Calibration started for user %d\n", epochTime, nId);
    }
    // Callback: Finished calibration
    void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* /*pCookie*/)
    {
        XnUInt32 epochTime = 0;
        xnOSGetEpochTime(&epochTime);
        if (eStatus == XN_CALIBRATION_STATUS_OK)
        {
            // Calibration succeeded
            printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
            g_UserGenerator.GetSkeletonCap().StartTracking(nId);
            
            if (gCallbackFunc) gCallbackFunc(sensor::Message::NewUser, nId);
        }
        else
        {
            // Calibration failed
            printf("%d Calibration failed for user %d\n", epochTime, nId);
            if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
            {
                printf("Manual abort occured, stop attempting to calibrate!");
                return;
            }
            if (g_bNeedPose)
            {
                g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
            }
            else
            {
                g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
            }
        }
    }
}


namespace
{
    void XN_CALLBACK_TYPE MyCalibrationInProgress(xn::SkeletonCapability& capability, XnUserID id, XnCalibrationStatus calibrationError, void* pCookie)
    {
        m_Errors[id].first = calibrationError;
    }
    
    void XN_CALLBACK_TYPE MyPoseInProgress(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID id, XnPoseDetectionStatus poseError, void* pCookie)
    {
        m_Errors[id].second = poseError;
    }
}


#define SAMPLE_XML_PATH "data/OpenNIConfig.xml"
#define XN_CALIBRATION_FILE_NAME "UserCalibration.bin"

#define CHECK_RC(nRetVal, what)                                        \
if (nRetVal != XN_STATUS_OK)                                    \
{                                                                \
printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));\
return nRetVal;                                                \
}

namespace
{
    bool initOpenNI()
    {
        XnStatus nRetVal = XN_STATUS_OK;
        
        xn::EnumerationErrors errors;
        nRetVal = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);
        if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
        {
            XnChar strError[1024];
            errors.ToString(strError, 1024);
            printf("%s\n", strError);
            return (nRetVal);
        }
        else if (nRetVal != XN_STATUS_OK)
        {
            printf("Open failed: %s\n", xnGetStatusString(nRetVal));
            return (nRetVal);
        }
        
        nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
        if (nRetVal != XN_STATUS_OK)
        {
            printf("No depth generator found. Using a default one...");
            xn::MockDepthGenerator mockDepth;
            nRetVal = mockDepth.Create(g_Context);
            CHECK_RC(nRetVal, "Create mock depth");
            
            // set some defaults
            XnMapOutputMode defaultMode;
            defaultMode.nXRes = 320;
            defaultMode.nYRes = 240;
            defaultMode.nFPS = 30;
            nRetVal = mockDepth.SetMapOutputMode(defaultMode);
            CHECK_RC(nRetVal, "set default mode");
            
            // set FOV
            XnFieldOfView fov;
            fov.fHFOV = 1.0225999419141749;
            fov.fVFOV = 0.79661567681716894;
            nRetVal = mockDepth.SetGeneralProperty(XN_PROP_FIELD_OF_VIEW, sizeof(fov), &fov);
            CHECK_RC(nRetVal, "set FOV");
            
            XnUInt32 nDataSize = defaultMode.nXRes * defaultMode.nYRes * sizeof(XnDepthPixel);
            XnDepthPixel* pData = (XnDepthPixel*)xnOSCallocAligned(nDataSize, 1, XN_DEFAULT_MEM_ALIGN);
            
            nRetVal = mockDepth.SetData(1, 0, nDataSize, pData);
            CHECK_RC(nRetVal, "set empty depth map");
            
            g_DepthGenerator = mockDepth;
        }
        
        nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
        if (nRetVal != XN_STATUS_OK)
        {
            nRetVal = g_UserGenerator.Create(g_Context);
            CHECK_RC(nRetVal, "Find user generator");
        }
        
        nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_ImageGenerator);
        if (nRetVal != XN_STATUS_OK)
        {
            nRetVal = g_ImageGenerator.Create(g_Context);
            CHECK_RC(nRetVal, "Find image generator..... HELP!!! ");
        }

        
        
        XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected, hCalibrationInProgress, hPoseInProgress;
        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
        {
            printf("Supplied user generator doesn't support skeleton\n");
            return 1;
        }
        nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
        CHECK_RC(nRetVal, "Register to user callbacks");
        nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
        CHECK_RC(nRetVal, "Register to calibration start");
        nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
        CHECK_RC(nRetVal, "Register to calibration complete");
        
        if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
        {
            g_bNeedPose = TRUE;
            if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
            {
                printf("Pose required, but not supported\n");
                return 1;
            }
            nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
            CHECK_RC(nRetVal, "Register to Pose Detected");
            g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
            
            nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseInProgress(MyPoseInProgress, NULL, hPoseInProgress);
            CHECK_RC(nRetVal, "Register to pose in progress");
        }
        
        g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
        
        nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationInProgress(MyCalibrationInProgress, NULL, hCalibrationInProgress);
        CHECK_RC(nRetVal, "Register to calibration in progress");
        
        nRetVal = g_Context.StartGeneratingAll();
        CHECK_RC(nRetVal, "StartGenerating");
        
        return nRetVal;
    }
    
    
    void shutdownOpenNI()
    {
        gCallbackFunc = nullptr;
        
        g_scriptNode.Release();
        g_DepthGenerator.Release();
        g_UserGenerator.Release();
        g_ImageGenerator.Release();
        g_Player.Release();
        g_Context.Release();
    }
}


struct SensorSysHelper
{
    ~SensorSysHelper()
    {
        shutdownOpenNI();
    }
} gssh;


namespace sensor
{
    void start(Callback func)
    {
        if (XN_STATUS_OK != initOpenNI())
        {
            //exit(-1);
            return;
        }
        
        g_Initialized = true;
        gCallbackFunc = func;
    }
    
    bool initialized() { return g_Initialized; }
    
    void updateAll()
    {
        if (!g_Initialized) return;
        
        g_Context.WaitAnyUpdateAll();
        //g_Context.WaitOneUpdateAll(g_UserGenerator);
    }
    
    xn::DepthGenerator &depthGenerator()
    {
        return g_DepthGenerator;
    }
    
    xn::UserGenerator &userGenerator()
    {
        return g_UserGenerator;
    }
    
    xn::ImageGenerator &imageGenerator()
    {
        return g_ImageGenerator;
    }
}
