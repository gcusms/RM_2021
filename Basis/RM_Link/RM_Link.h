#pragma once
#include "Basis/RM_Kalman/RM_Kalman.h"
#include "Basis/RM_Serial/RM_Serial.h"
#include "Basis/RM_SolvePnP/RM_SolvePnP.h"
#include "Basis/RM_VideoCapture/RM_VideoCapture.h"
#include "Tools/RM_DataExchange/RM_DataExchange.h"
#include "Tools/RM_FPS/RM_FPS.h"
#include "Tools/RM_Messenger/RM_Messenger.h"
#include <memory>
#include <opencv4/opencv2/opencv.hpp>
using namespace cv;

class RM_Link {
  public:
    RM_Link();
    ~RM_Link();

    // 总运行函数
    void run();

  private:
    /* Tools */
    /* 计算帧率 */
    unique_ptr<RM_FPS> fps;
    /* 录制视频 TODO 添加开关*/
    /* 文件数据交换 */
    unique_ptr<RM_DataExchange> data_exchange;

  private:
    //各部分对象声明
    /* Basis长期启动 */
    /* 工业相机 */
    unique_ptr<RM_VideoCapture> industrialcapture;
    /* USB相机 */
    unique_ptr<VideoCapture> capture;
    /* 角度解算器 */
    unique_ptr<RM_SolvePnP> solver;
    /* 串口通讯 */
    unique_ptr<SerialPort> serialport;

    /* 卡尔曼预测 TODO*/
    /* 传递发送信息 */
    unique_ptr<RM_Messenger> messenger;

  private:
    /* Model TODO*/
    /* 自瞄 TODO*/
    unique_ptr<RM_Armor> armor;
    /* 能量机关 TODO*/

  private:
    //公共原图
    unique_ptr<Mat> frame;
    unique_ptr<Mat> src_img;
};
