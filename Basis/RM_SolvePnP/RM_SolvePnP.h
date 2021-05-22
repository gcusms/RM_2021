#pragma once
#include "Basis/RM_Control/Debug_Controller.h"
#include "Basis/RM_Control/Release_Controller.h"
#include "Tools/RM_Messenger/RM_Messenger.h"
#include <opencv4/opencv2/opencv.hpp>

#define DEBUG

typedef struct PNP_Config
{
    // controler
    int IS_DRAW_COORDINATE;
    int SHOW_PNP_INFORMATION;

    // param
    std::string CAMERA_PARAM_FILE;  // 标定数据
    float       CAMERA_PTZ_X;
    float       CAMERA_PTZ_Y;
    float       CAMERA_PTZ_Z;
    float       PTZ_BARREL_X;
    float       PTZ_BARREL_Y;
    float       PTZ_BARREL_Z;
    int         BIG_ARMOR_SIZE_W;
    int         BIG_ARMOR_SIZE_H;
    int         SMALL_ARMOR_SIZE_W;
    int         SMALL_ARMOR_SIZE_H;
    int         BUFF_ARMOR_SIZE_W;
    int         BUFF_ARMOR_SIZE_H;
    double      SCALE_Z;  // TODO

    // offset
    float OFFSET_ARMOR_PITCH;   // pitch offset
    float OFFSET_ARMOR_YAW;     // yaw offset
    int   OFFSET_PITCH_SYMBOL;  // pitch symbol 0:- 1:+
    int   OFFSET_YAW_SYMBOL;    // yaw symbol 0:- 1:+

    // 能量机关
    float BUFF_GROUND_H;  // mm
    float ROBOT_H;        // mm
    float UPLAND_H;       // mm
    float BUFF_ROBOT_Z;   // mm

    PNP_Config()
    {
        IS_DRAW_COORDINATE   = 0;
        SHOW_PNP_INFORMATION = 0;
        CAMERA_PTZ_X         = 0.f;
        CAMERA_PTZ_Y         = 45.5;
        CAMERA_PTZ_Z         = 68.9;
        PTZ_BARREL_X         = 0.f;
        PTZ_BARREL_Y         = 0.f;
        PTZ_BARREL_Z         = 0.f;
        BIG_ARMOR_SIZE_W     = 250;
        BIG_ARMOR_SIZE_H     = 65;
        SMALL_ARMOR_SIZE_W   = 140;
        SMALL_ARMOR_SIZE_H   = 60;
        BUFF_ARMOR_SIZE_W    = 250;  // TODO:Need to test
        BUFF_ARMOR_SIZE_H    = 65;   // TODO:Need to test

        BUFF_GROUND_H = 1586;
        ROBOT_H       = 330;
        UPLAND_H      = 850;
        BUFF_ROBOT_Z  = 6915.340249311;
    }
} PNP_Cfg;

class RM_SolvePnP {
  public:
    RM_SolvePnP(const PNP_Cfg& _pnp_config);
    ~RM_SolvePnP();

    // 初始化
    // 设置相机和云台的转换关系（矩阵）
    void setRelationPoseCameraPTZ();
    // 设置真实3d点
    void set3dPoint();

    // 调整目标矩形（像素）的宽高

    // 异常处理

    // 外部接口 获取：矩形顶点、弹速、类型、画图板
    void runSolvePnP(const std::vector<cv::Point2f>& _point_2d,
                     const armor_size&               _type,
                     cv::Mat&                        _src_img);

    // 手动补偿 TODO
    void offsetManually(RM_Messenger* _messenger);

    // 变换坐标系 相机到云台
    void transformCamera2PTZ(const cv::Mat& _pos_in_camera, cv::Mat& _transed_pos);

    // 变换坐标系 云台到枪管 并获取角度
    void getAngle(RM_Messenger* _messenger, float yaw);

    void getAngle_buff(RM_Messenger* _messenger, const float _buff_angle);

  private:
    // 重置 TODO
    void reset();
    // 绘制 3d 坐标系
    void drawCoordinate(cv::Mat& input_img);
    // 修正z轴数据（深度）TODO
    void repairDepthInfomation(float& _depth);
    // 修正pitch
    float getPitch(double* xyz, float dist, float tvec_y, float _bullet_velocity);
    // 修正pitch（方法介绍：华工） DEBUG

  private:
    // 结构体
    PNP_Cfg pnp_cfg;
    // camera
    cv::Mat camera_matrix;                                             // 相机内参
    cv::Mat distortion_coeff;                                          // 畸变矩阵
    cv::Mat position_in_camera_rvec = cv::Mat::zeros(3, 3, CV_64FC1);  // 旋转矩阵
    cv::Mat position_in_camera      = cv::Mat::zeros(3, 1, CV_64FC1);  // 位移向量

    // ptz
    cv::Mat position_in_ptz = cv::Mat::zeros(3, 1, CV_64FC1);  // 位移向量

    float overlap_dist = 100000.f;

    // camera-ptz 相机坐标系转云台坐标系
    cv::Mat trans_camera2ptz;  // 位移向量
    cv::Mat rot_camera2ptz;    // 旋转矩阵

    // Depth compensation coefficient 深度补偿系数 TODO
    // double scale_z;

    double width_target;   // 目标实际宽度 （mm）
    double height_target;  // 目标实际高度 （mm）

    std::vector<cv::Point3f> big_armor_3d;    // 目标真实顶点
    std::vector<cv::Point3f> small_armor_3d;  // 目标真实顶点
    std::vector<cv::Point3f> buff_armor_3d;   // 目标真实顶点
    // std::vector<cv::Point2f> target_2d;  // 目标图像顶点

    int type;  // 解算类型

  private:
    // 绘制 3d 坐标系
    std::vector<cv::Point2f> reference_Img;
    std::vector<cv::Point3f> reference_Obj;

  private:
    // 重力补偿
    double down_t;  // s
    double offset_gravity;
    double alpha;
    double beta;

    float thta;
    float balta;

    int     offset_armor_pitch;
    int     offset_armor_yaw;
    int     offset_pitch_symbol;
    int     offset_yaw_symbol;
    cv::Mat offset_trackbar_img = cv::Mat::zeros(1, 1200, CV_8UC1);

  private:
    // 能量机关
    float delta_h;   // 枪口距离能量机关最低装甲板的高度差
    float buff_h;    // 计算风车相对最底面装甲高度　０－１６００
    float target_h;  // 最终目标的高度
    float distance;  // 模型计算的目标深度
};
