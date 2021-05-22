#pragma once

/*---工业相机中使用到opencv2.0的 IplImage 需要包含此头文件 ---*/
#include <CameraApi.h>
/*---工业相机中使用到opencv2.0的 cvReleaseImageHeader 需要包含此头文件 ---*/
#include "Basis/RM_Control/Debug_Controller.h"
// #include "Tools/RM_DataExchange/RM_DataExchange.h"
#include <iostream>
#include <memory>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv4/opencv2/opencv.hpp>
#include"Basis/RM_Control/Release_Controller.h"

using namespace std;

// 相机
typedef struct Camera_Config
{
    int ISOPEN_INDUSTRY_CAPTURE;

    // TODO:完善数据分类
    int CAMERA_EXPOSURETIME;
    int CAMERA_RESOLUTION_COLS;
    int CAMERA_RESOLUTION_ROWS;
    int CAMERA_RESOLUTION_COLS_FOV;
    int CAMERA_RESOLUTION_ROWS_FOV;
    int CAMERA_BLUE_GAIN;
    int CAMERA_GREEN_GAIN;
    int CAMERA_RED_GAIN;
    int CAMERA_GAMMA;
    int CAMERA_CONTRAST;
    int CAMERA_SATURATION;
    int CAMERA_CONFIG;
    int IS_PARAM_ADJUSTMENT;
    Camera_Config()
    {
        ISOPEN_INDUSTRY_CAPTURE = 0;

        CAMERA_EXPOSURETIME        = 400;
        CAMERA_RESOLUTION_COLS     = 1280;
        CAMERA_RESOLUTION_ROWS     = 800;
        CAMERA_RESOLUTION_COLS_FOV = ((1280 - CAMERA_RESOLUTION_COLS) * 0.5);
        CAMERA_RESOLUTION_ROWS_FOV = ((1024 - CAMERA_RESOLUTION_ROWS) * 0.5);
        CAMERA_BLUE_GAIN           = 123;
        CAMERA_GREEN_GAIN          = 123;
        CAMERA_RED_GAIN            = 103;
        CAMERA_GAMMA               = 50;
        CAMERA_CONTRAST            = 83;
        CAMERA_SATURATION          = 166;
        CAMERA_CONFIG              = 1;
        IS_PARAM_ADJUSTMENT        = 0;
    }
} Camera_Cfg;

class RM_VideoCapture {
  public:
    unsigned char* g_pRgbBuffer;  //处理后数据缓存区

    int                 iCameraCounts = 1;
    int                 iStatus       = -1;
    tSdkCameraDevInfo   tCameraEnumList;
    int                 hCamera;
    tSdkCameraCapbility tCapability;  //设备描述信息
    tSdkFrameHead       sFrameInfo;
    BYTE*               pbyBuffer;
    IplImage*           iplImage = nullptr;
    int                 channel  = 3;
    BOOL                AEstate  = FALSE;
    tSdkImageResolution pImageResolution;  //相机分辨率信息

    bool iscamera0_open = false;

  public:
    RM_VideoCapture(const Camera_Cfg& _camera_config);
    ~RM_VideoCapture();
    bool isindustryimgInput();
    void cameraReleasebuff();
    void cameraSet();

  private:
    Camera_Cfg camera_cfg;

  private:
    // 滑动条窗口创建
    cv::Mat camera_config = cv::Mat::zeros(1, 1200, CV_8UC1);  // TODO：可嵌入初始化范围内

    int exposuretime = this->camera_cfg.CAMERA_EXPOSURETIME;

};
