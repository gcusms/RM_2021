#include "Basis/RM_VideoCapture/RM_VideoCapture.h"

/**
 * @brief Construct a new rm videocapture::rm videocapture object
 *        设置 0 为工业相机的输入编号
 *
 * @param cameramode 相机型号
 */
RM_VideoCapture::RM_VideoCapture(const Camera_Cfg& _camera_config)
{
    this->camera_cfg = _camera_config;
    if (this->camera_cfg.ISOPEN_INDUSTRY_CAPTURE == 0) {
        cameraSet();
        iscamera0_open = true;
        cout << "Set camera Industrial camera✔️ ✔️ ✔️ ✔️ ✔️" << endl;
    }
    else {
        iscamera0_open = false;
        cout << "set camera USB camera" << endl;
    }
}

/**
 * @brief Destroy the rm videocapture::rm videocapture object
 *
 */
RM_VideoCapture::~RM_VideoCapture()
{
    if (iscamera0_open) {
        CameraUnInit(hCamera);
        //注意，现反初始化后再free
        free(g_pRgbBuffer);
        cout << "release Industry camera success......  🤪🤪" << endl;
    }
    else {
        cout << "release USB camera success......  🤪🤪" << endl;
    }
}

/**
 * @brief 工业相机初始化
 * @return int
 */
void RM_VideoCapture::cameraSet()
{
    CameraSdkInit(1);
    //枚举设备，并建立设备列表
    iStatus = CameraEnumerateDevice(&tCameraEnumList, &iCameraCounts);
    printf("state = %d\n", iStatus);
    printf("count = %d\n", iCameraCounts);
    //没有连接设备
    if (iCameraCounts == 0) {
        cout << "没有连接设备" << endl;
        return;
    }
    //相机初始化。初始化成功后，才能调用任何其他相机相关的操作接口
    iStatus = CameraInit(&tCameraEnumList, -1, -1, &hCamera);
    //初始化失败
    printf("state = %d\n", iStatus);
    if (iStatus != CAMERA_STATUS_SUCCESS) {
        cout << "初始化失败" << endl;
        return;
    }
    //获得相机的特性描述结构体。该结构体中包含了相机可设置的各种参数的范围信息。决定了相关函数的参数
    CameraGetCapability(hCamera, &tCapability);
    g_pRgbBuffer = (unsigned char*)malloc(tCapability.sResolutionRange.iHeightMax *
                                          tCapability.sResolutionRange.iWidthMax * 3);  // malloc

    /*--------设置分辨率---------*/
    CameraGetImageResolution(hCamera, &pImageResolution);
    pImageResolution.iIndex      = 0xFF;
    pImageResolution.iWidthFOV   = this->camera_cfg.CAMERA_RESOLUTION_COLS;
    pImageResolution.iHeightFOV  = this->camera_cfg.CAMERA_RESOLUTION_ROWS;
    pImageResolution.iWidth      = this->camera_cfg.CAMERA_RESOLUTION_COLS;
    pImageResolution.iHeight     = this->camera_cfg.CAMERA_RESOLUTION_ROWS;
    pImageResolution.iHOffsetFOV = int(this->camera_cfg.CAMERA_RESOLUTION_COLS_FOV);
    pImageResolution.iVOffsetFOV = int(this->camera_cfg.CAMERA_RESOLUTION_ROWS_FOV);
    CameraSetImageResolution(hCamera, &pImageResolution);
    /*--------设置分辨率---------*/

    /*--------设置曝光时间---------*/
    cout << CameraGetAeState(hCamera, &AEstate);
    cout << CameraSetAeState(hCamera, FALSE);
    // TODO:视情况更改相应颜色的曝光值
    if (this->camera_cfg.CAMERA_CONFIG == 0 || this->camera_cfg.CAMERA_CONFIG == 1) {
        CameraSetExposureTime(hCamera, this->camera_cfg.CAMERA_EXPOSURETIME);
    }

    /*--------设置曝光时间---------*/

    /*----- 伽马值、饱和度、对比度、颜色增益 -----*/
    if (this->camera_cfg.CAMERA_CONFIG == 1) {
        CameraSetGain(hCamera, this->camera_cfg.CAMERA_RED_GAIN, this->camera_cfg.CAMERA_GREEN_GAIN,
                      this->camera_cfg.CAMERA_BLUE_GAIN);
        CameraSetGamma(hCamera, this->camera_cfg.CAMERA_GAMMA);
        CameraSetContrast(hCamera, this->camera_cfg.CAMERA_CONTRAST);
        CameraSetSaturation(hCamera, this->camera_cfg.CAMERA_SATURATION);
    }
    /*----- 伽马值、饱和度、对比度、颜色增益 -----*/

    /*让SDK进入工作模式，开始接收来自相机发送的图像数据。
     *如果当前相机是触发模式，则需要接收到触发帧以后才会更新图像*/
    CameraPlay(hCamera);
    CameraReleaseImageBuffer(hCamera, pbyBuffer);
    /*
  其他的相机参数设置
  例如 CameraSetExposureTime   CameraGetExposureTime  设置/读取曝光时间
       CameraSetImageResolution  CameraGetImageResolution 设置/读取分辨率
       CameraSetGamma、CameraSetConrast、CameraSetGain等设置图像伽马、对比度、RGB数字增益等等。
       更多的参数的设置方法，，清参考MindVision_Demo。本例程只是为了演示如何将SDK中获取的图像，转成OpenCV的图像格式,以便调用OpenCV的图像处理函数进行后续开发
  */
    if (tCapability.sIspCapacity.bMonoSensor) {
        channel = 1;
        CameraSetIspOutFormat(hCamera, CAMERA_MEDIA_TYPE_MONO8);
    }
    else {
        channel = 3;
        CameraSetIspOutFormat(hCamera, CAMERA_MEDIA_TYPE_BGR8);
    }
    return;
}

/**
 * @brief 工业相机的图像转到指针中 再通过指针转换变为 Mat
 * @return true 成功启用工业相机
 * @return false 不能启用工业相机
 */
bool RM_VideoCapture::isindustryimgInput()
{
#ifndef RELEASE
    /* -----设置伽马值----- */
    if ((this->camera_cfg.CAMERA_CONFIG == 2 || this->camera_cfg.CAMERA_CONFIG == 3) &&
        this->camera_cfg.IS_PARAM_ADJUSTMENT == 1) {
        cv::namedWindow("Camera");
        cv::createTrackbar("EXPOSURETIME", "Camera", &exposuretime, 10000, nullptr);
        CameraSetExposureTime(hCamera, exposuretime);

        if (this->camera_cfg.CAMERA_CONFIG == 3) {
            cv::createTrackbar("BLUE_gain", "Camera", &this->camera_cfg.CAMERA_BLUE_GAIN, 400,
                               nullptr);
            cv::createTrackbar("GREEN_gain", "Camera", &this->camera_cfg.CAMERA_GREEN_GAIN, 400,
                               nullptr);
            cv::createTrackbar("RED_gain", "Camera", &this->camera_cfg.CAMERA_RED_GAIN, 400,
                               nullptr);
            cv::createTrackbar("Gamma", "Camera", &this->camera_cfg.CAMERA_GAMMA, 240, nullptr);
            cv::createTrackbar("Contrast", "Camera", &this->camera_cfg.CAMERA_CONTRAST, 200,
                               nullptr);
            cv::createTrackbar("Saturation", "Camera", &this->camera_cfg.CAMERA_SATURATION, 190,
                               nullptr);

            CameraSetGain(hCamera, this->camera_cfg.CAMERA_RED_GAIN,
                          this->camera_cfg.CAMERA_GREEN_GAIN, this->camera_cfg.CAMERA_BLUE_GAIN);
            CameraSetGamma(hCamera, this->camera_cfg.CAMERA_GAMMA);
            CameraSetContrast(hCamera, this->camera_cfg.CAMERA_CONTRAST);
            CameraSetSaturation(hCamera, this->camera_cfg.CAMERA_SATURATION);
        }
        cv::imshow("Camera", camera_config);
    }
/* -----设置伽马值----- */
#endif  // !RELEASE

    bool isindustry_camera_open = false;
    if (iscamera0_open == 1) {
        if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS) {
            //----------读取原图----------//
            CameraImageProcess(hCamera, pbyBuffer, g_pRgbBuffer, &sFrameInfo);
            if (iplImage) {
                cvReleaseImageHeader(&iplImage);
            }
            iplImage = cvCreateImageHeader(cvSize(sFrameInfo.iWidth, sFrameInfo.iHeight),
                                           IPL_DEPTH_8U, channel);
            cvSetData(iplImage, g_pRgbBuffer, sFrameInfo.iWidth * channel);
            //此处只是设置指针，无图像块数据拷贝，不需担心转换效率
        }
        return isindustry_camera_open = true;
    }
    else {
        return isindustry_camera_open = false;
    }
}

/**
 * @brief 释放相机缓存数据
 *
 */
void RM_VideoCapture::cameraReleasebuff()
{
    if (iscamera0_open) {
        //在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
        //否则再次调用CameraGetImageBuffer时，程序将被挂起一直阻塞，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
        CameraReleaseImageBuffer(hCamera, pbyBuffer);
    }
}
