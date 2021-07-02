/**
 * @file rm_videocapture.h
 * @author GCUROBOT-Visual-Group (GCUROBOT_WOLF@163.com)
 * @brief RM 2019 步兵视觉相机配置头文件
 * @version 1.1
 * @date 2019-05-06
 * @copyright Copyright (c) 2019 GCU Robot Lab. All Rights Reserved.
 */
#ifndef RM_VIDEOCAPTURE_H_
#define RM_VIDEOCAPTURE_H_

#include "mv_camera_param.h"

#include <CameraApi.h>
/*---工业相机中使用到 opencv2.0 的 IplImage 需要包含此头文件 ---*/
#include <opencv2/imgproc/imgproc_c.h>
/*---工业相机中使用到 opencv2.0 的 IplImage 需要包含此头文件 ---*/
#include <opencv2/opencv.hpp>

namespace mv_camera {

class RM_VideoCapture
{
private:
  unsigned char           *g_pRgbBuffer; //处理后数据缓存区

  int                     iCameraCounts = 1;
  int                     iStatus=-1;
  tSdkCameraDevInfo       tCameraEnumList;
  int                     hCamera;
  tSdkCameraCapbility     tCapability;      //设备描述信息
  tSdkFrameHead           sFrameInfo;
  BYTE                    *pbyBuffer;
  IplImage                *iplImage = nullptr;
  int                     channel=3;
  BOOL                    AEstate=FALSE;
  tSdkImageResolution     pImageResolution;       //相机分辨率信息

  bool                    iscamera0_open = false;

public:
  /**
   * @brief Construct a new RM_VideoCapture::RM_VideoCapture object
   * @param[in] _camera_param 相机参数结构体
   */
  RM_VideoCapture(const CameraParam &_camera_param);
  
  /**
   * @brief Destroy the rm videocapture::rm videocapture object
   */
  ~RM_VideoCapture();
  
  /**
   * @brief 工业相机的图像转到指针中 再通过指针转换变为 Mat
   * @return true 成功启用工业相机
   * @return false 不能启用工业相机
   */
  bool isindustryimgInput();
  
  /**
   * @brief 释放相机缓存数据
   */
  void cameraReleasebuff();
  
  int cameraInit( const int _CAMERA_RESOLUTION_COLS,
                  const int _CAMERA_RESOLUTION_ROWS,
                  const int _CAMERA_EXPOSURETIME);

  inline cv::Mat image() const { 
    return cv::cvarrToMat(this->iplImage ,true);
  }
};

} // namespace mv_camera

#endif // !RM_VIDEOCAPTURE_H_