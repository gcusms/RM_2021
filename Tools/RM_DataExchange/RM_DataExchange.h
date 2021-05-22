#pragma once

#include "Basis/RM_Control/Debug_Controller.h"
#include "Basis/RM_Serial/RM_Serial.h"
#include "Basis/RM_SolvePnP/RM_SolvePnP.h"
#include "Basis/RM_VideoCapture/RM_VideoCapture.h"
#include "Model/RM_Armor/RM_Armor.h"
#include "Model/RM_Buff/RM_Buff.h"
#include "Tools/RM_Messenger/RM_Messenger.h"
#include "Tools/RM_Roi/RM_Roi.h"
#include <opencv4/opencv2/opencv.hpp>
#include <string.h>

class RM_DataExchange {
  public:
    // 构造函数
    RM_DataExchange(const std::string& filename);
    ~RM_DataExchange();
    // 读取函数
    void readData();
    // 写入函数
    void writeData();
    // 返回videocapture的值是文件还是相机序号

  public:
    // 存放的结构体
    Setting_Cfg setting_cfg;
    // 相机的储存信息
    Camera_Cfg camera_cfg;
    // 自瞄信息
    Armor_Cfg armor_cfg;
    // pnp信息
    PNP_Cfg pnp_cfg;
    // 大神符消息TODO
    Buff_Cfg buff_cfg;
    // Messenger
    Messenger_Cfg messneger_cfg;
    //串口
    Serial_Cfg serial_cfg;
    // Roi
    Roi_Cfg roi_cfg;

  private:
    // 读取
    void read(const cv::FileStorage& fs);
    // 写入
    void write(cv::FileStorage& fs) const;
    // TODO:检查 check + 默认配置结构体的初始化
    void check();

    // 更新控制开关数据
    void updateSettingInfo(const cv::FileStorage& fs);
    // 更新相机数据
    void updateCameraConfig(const cv::FileStorage& fs);
    // 更新自瞄数据
    void updateArmorConfig(const cv::FileStorage& fs);
    // 更新pnp数据
    void updatePNPConfig(const cv::FileStorage& fs);
    // TODO:更新能量机关数据
    void updateBuffConfig(const cv::FileStorage& fs);
    // 更新信使类数据
    void updateMessengerConfig(const cv::FileStorage& fs);
    // 更新串口数据
    void updateSerialConfig(const cv::FileStorage& fs);
    // 更新Roi数据
    void updateRoiConfig(const cv::FileStorage& fs);

    // TODO：设置Release模式的参数
    void resetSettingInfo();
    void resetCameraConfig();
    void resetArmorConfig();
    void resetPNPConfig();
    void resetBuffConfig();
    void resetMessengerConfig();
    void resetSerialConfig();
    void resetRoiConfig();

  private:
    // 存放数据文件的地址
    std::string file;
};