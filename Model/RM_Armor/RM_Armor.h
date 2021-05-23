#pragma once

#include "Basis/RM_Control/Debug_Controller.h"
#include "Basis/RM_Control/Release_Controller.h"
#include "Tools/RM_FPS/RM_FPS.h"
#include "Tools/RM_Messenger/RM_Messenger.h"
#include "Tools/RM_Roi/RM_Roi.h"
#include <algorithm>
#include <opencv4/opencv2/opencv.hpp>

using namespace cv;

// debug测试的时候开启
// #define DEBUG

// 预处理的模式
enum Processing_Mode {
    BGR_MODE,
    HSV_MODE,
    LAB_MODE,
};

// 计算中点距离
float centerDistance(const Point& p1, const Point& p2);

//灯条类配置文件
typedef struct Light_Config
{
    // 灯条
    float RATIO_W_H_MAX;     // 宽高比 Max
    float RATIO_W_H_MIN;     // 宽高比 Min
    int   LIGHT_AREA;        // 灯条面积（舍弃）
    int   LIGHT_HEIGHT;      // 灯条高度
    int   LIGHT_LENGTH_MIN;  // 灯条周长 Min
    int   LIGHT_LENGTH_MAX;  // 灯条周长 Max

} Light_Cfg;

//候选装甲板类配置文件
typedef struct Candidate_Armor_Config
{
    // 装甲板
    float DIFF_LIGHT_Y;          // 灯条高度差
    float DISTANCE_MULTIPLE_TH;  // 灯条水平距离相差倍数阈值
    int   DISTANCE_MIN_TH;       // 灯条水平距离最小值
    float DIFF_ANGLE_TH_SAME;    // 两灯条角度差 - 同侧
    float LIGHT_SIZE_RATIO_MAX;  // 灯条大小比例 Max
    float LIGHT_SIZE_RATIO_MIN;  // 灯条大小比例 Min
    float ARMOR_TYPE_TH;         // 装甲板类型判断阈值
    int   MATCHING_TH;           // 双指针跨越匹配阈值

} Candidate_Armor_Cfg;

// 自瞄配置文件
typedef struct Armor_Config
{
    // 控制开关 见xml文件
    int IS_NUMBER_PREDICT_OPEN;
    int IS_ENABLE_ROI_RECT;
    int IS_DRAW_LIGHT;
    int IS_DRAW_ARMOR_RECT;
    int IS_PARAM_ADJUSTMENT;
    int IS_SHOW_BIN_IMG;
    int PROCESSING_MODE;

    // 参数
    int     CAMERA_RESOLUTION_ROWS;
    int     CAMERA_RESOLUTION_COLS;
    int     CAMERA_RESOLUTION_COLS_FOV;
    int     CAMERA_RESOLUTION_ROWS_FOV;
    Point2f CAMERA_RESOLUTION_FOV;
    int     FIRST_LOSE_CNT_MAX;
    int     SECOND_LOSE_CNT_MAX;
    int     FIRST_LEVEL_LOSS_RANGE;
    int     SECOND_LEVEL_LOSS_RANGE;

    // BGR
    int RED_ARMOR_GRAY_TH;
    int RED_ARMOR_COLOR_TH;
    int BLUE_ARMOR_GRAY_TH;
    int BLUE_ARMOR_COLOR_TH;
    // HSV-red
    int H_RED_MAX;
    int H_RED_MIN;
    int S_RED_MAX;
    int S_RED_MIN;
    int V_RED_MAX;
    int V_RED_MIN;
    // HSV-blue
    int H_BLUE_MAX;
    int H_BLUE_MIN;
    int S_BLUE_MAX;
    int S_BLUE_MIN;
    int V_BLUE_MAX;
    int V_BLUE_MIN;

    // 筛选条件
    Light_Cfg           light_cfg;
    Candidate_Armor_Cfg candidate_armor_cfg;

    // 初始化数据，Release版本使用以下默认参数
    Armor_Config()
    {
        // arrmor
        IS_NUMBER_PREDICT_OPEN = 0;
        IS_ENABLE_ROI_RECT     = 1;
        IS_DRAW_LIGHT          = 0;
        IS_DRAW_ARMOR_RECT     = 0;
        IS_PARAM_ADJUSTMENT    = 0;
        IS_SHOW_BIN_IMG        = 0;
        PROCESSING_MODE        = 1;

        CAMERA_RESOLUTION_COLS     = 1280;
        CAMERA_RESOLUTION_ROWS     = 800;
        CAMERA_RESOLUTION_COLS_FOV = ((1280 - CAMERA_RESOLUTION_COLS) * 0.5);
        CAMERA_RESOLUTION_ROWS_FOV = ((1024 - CAMERA_RESOLUTION_ROWS) * 0.5);
    }
} Armor_Cfg;

// 灯条类
class Light {
  public:
    Light(){};
    ~Light(){};
    // 更新数据
    void inputparam(const cv::RotatedRect& _rect,
                    const size_t&          _index,
                    const vector<Point>&   _contours,
                    RM_ROI&                _roi,
                    const Point&           _img_center = Point(640, 450));
    // 更新索引
    void updateIndex(const size_t& _index);

    // 返回矩形
    inline cv::RotatedRect returnRect() const
    {
        return this->rect;
    }

    // 返回宽
    inline int returnWidth() const
    {
        return this->width;
    }

    // 返回高
    inline int returnHeight() const
    {
        return this->height;
    }

    // 返回高宽比
    inline float returnAspectRatio()
    {
        return this->aspect_ratio;
    }

    // 返回角度
    inline float returnAngle() const
    {
        return this->angle;
    }

    // 返回周长
    inline int returnPerimeter()
    {
        return this->perimeter;
    }

    // 返回索引号
    inline size_t returnIndex()
    {
        return this->index;
    }

    // 返回相应顶点
    inline cv::Point2f returnVertex(const int& i)
    {
        return this->vertex[i];
    }

    // 返回顶点
    inline cv::Point2f* returnVertex()
    {
        return this->vertex;
    }
    // 返回距离图像中心距离
    inline float returnDistToCenter()
    {
        return this->dist_to_center;
    }

  private:
    // 计算中点距离（友元）
    friend float centerDistance(const Point& p1, const Point& p2);

  private:
    // 灯条矩形
    cv::RotatedRect rect = cv::RotatedRect();
    // 灯条序号 rotaterect 给的index是随机的
    size_t index;
    // 宽高比
    float aspect_ratio;
    // 角度
    float angle;
    // 宽
    int width;
    // 高
    int height;
    // 周长
    int perimeter;
    // 顶点
    cv::Point2f vertex[4];
    // 装甲板中心到图像中心的距离
    float dist_to_center;
};

// 候选装甲板类
class CandidateArmor {
  public:
    CandidateArmor(){};

    CandidateArmor(const Candidate_Armor_Cfg& _candidate_armor_config);

    ~CandidateArmor(){};

    void inputparam(const Light&  _left_light,
                    const Light&  _right_light,
                    const size_t& _index,
                    RM_ROI&       _roi,
                    const Point&  _img_center = Point(640, 450));

    // 返回装甲板矩形
    inline RotatedRect returnRect()
    {
        return this->rect;
    }

    // 返回角度
    inline float returnAngle()
    {
        return this->angle;
    }

    // 返回宽高比
    inline float returnAspectRatio()
    {
        return this->aspect_ratio;
    }

    // 返回大小装甲标志
    inline armor_size returnArmorType()
    {
        return this->type;
    }

    // 返回距离图像中心距离
    inline float returnDistToCenter()
    {
        return this->dist_to_center;
    }

    // 返回索引号
    inline size_t returnIndex()
    {
        return this->index;
    }

    // 返回角度差
    inline float returnDiffAngle()
    {
        return this->diff_angle;
    }

    // 返回宽
    inline int retunrWidth()
    {
        return this->width;
    }

    // 返回高
    inline int returnHeight()
    {
        return this->height;
    }

    // 返回顶点
    inline cv::Point2f returnVertex(const int& i)
    {
        return this->vertex[i];
    }

    // 更新索引号
    inline void updateIndex(const size_t& _index)
    {
        this->index = _index;
    }

    // 返回左灯条
    inline Light& returnLeftLight()
    {
        return this->left_light;
    }

    // 返回右灯条
    inline Light& returnRightLight()
    {
        return this->right_light;
    }

  private:
    // 初步拟合装甲板
    cv::RotatedRect draftTentativeArmor(const Light& _left_light, const Light& _right_light);

    // 计算中点距离（友元）
    friend float centerDistance(const Point& p1, const Point& p2);

    // 更新信息
    void
    updateCandidateArmor(const Light& _left_light, const Light& _right_light, const size_t& _index);

    // 判断大小装甲
    armor_size judgeArmorSize();

  private:
    // 拟合装甲板参数配置表
    Candidate_Armor_Cfg candidate_armor_config;
    // flag TODO

    // 大小装甲判断
    armor_size type;
    float      armor_type_th;

    // 左灯条
    Light left_light;

    // 右灯条
    Light right_light;

    // 矩形
    cv::RotatedRect rect;

    // 宽高比
    float aspect_ratio;

    // 装甲板中心到图像中心的距离
    float dist_to_center;

    // 角度
    float angle;

    // 角度差
    float diff_angle;

    // 序号
    size_t index;

    // 宽
    float width;

    // 高
    float height;

    // 顶点
    Point2f vertex[4];

  private:
    // 初步拟合所需参数（可省略） TODO
    Point2f     center_tmp;
    float       distance_tmp;
    float       w_tmp;
    float       h_tmp;
    float       angle_tmp;
    RotatedRect rect_tmp;
};

// 自瞄类
class RM_Armor {
  public:
    void ArmorVertex(int a);
    RM_Armor(const Armor_Cfg& _armor_config, const Roi_Cfg& _roi_config);
    ~RM_Armor();

    // 总执行函数
    bool identificationTask(Mat _frame, RM_Messenger* _messenger);

    vector<Point2f> returnTarget2DPoint();

    CandidateArmor& returnFinalArmor();

    bool returnIsLastData();

  private:
    // 预处理
    void imageProcessing(Mat _frame, const int& _my_color);

    // 创建灯条筛选条件的滑动条
    void createLightTrackbar();
    // 创建装甲板筛选条件的滑动条
    void createCandidateArmorTrackbar();
    // 创建最优装甲板筛选条件的滑动条
    void createOpeimumArmorTrackbar();

    // 筛选灯条
    bool lightScreenting();

    // 筛选候选装甲板
    bool armorScreenting();

    // 选取最优装甲板
    bool armorSelectOptimum();

  private:
    // BGR处理
    void BGRProcessing(const int& _my_color);

    // HSV处理
    void HSVProcessing(const int& _my_color);

    Mat src_img;         // 预处理原图
    Mat roi_img;         // 预处理输入图
    Mat hsv_img;         // hsv 通道
    Mat dst_img;         // 装甲板识别最终图像
    Mat gray_img;        // 灰度图
    Mat bin_img_color;   // 颜色二值图
    Mat bin_img_color1;  // 颜色二值图一
    Mat bin_img_color2;  // 颜色二值图二
    Mat bin_img_gray;    // 灰度二值图

    // 椭圆内核
    Mat ele = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

    // BGR
    vector<Mat> split_img;   //分通道图
    int         average_th;  //平均阈值

    // HSV

  private:
    // 运算函数
    // 两点之间的距离
    friend float centerDistance(const Point& p1, const Point& p2);

    // 装甲板顶点排序
    void sortArmorVertex();
    
    float yaw = 0; 
  private:
    // 参数结构体
    Armor_Cfg armor_config;

  private:
#ifndef RELEASE
    // 滑动条窗口
    Mat trackbar_img               = Mat::zeros(1, 300, CV_8UC1);  // 预处理
    Mat light_trackbar_img         = Mat::zeros(1, 300, CV_8UC1);  // 灯条条件
    Mat armor_trackbar_img         = Mat::zeros(1, 300, CV_8UC1);  // 装甲板条件
    Mat optimum_armor_trackbar_img = Mat::zeros(1, 300, CV_8UC1);  // 最优装甲板条件
#endif                                                             // !RELEASE

  private:
    // roi 感兴趣区域
    RM_ROI armor_roi;

  private:
    // 灯条
    Light light, light_left, light_right;

    // 正确灯条
    vector<Light> light_box;  // TODO：预设大小

    // 二值化点集
    vector<vector<Point>> contours;

    RotatedRect rect_tmp;  // 临时变量

#ifndef RELEASE
    Mat light_img;  // 灯条筛选效果图
#endif              // !RELEASE

#ifndef RELEASE
    // trackbar
    int h_w_ratio_min_int = this->armor_config.light_cfg.RATIO_W_H_MIN * 100;  //转 int
    int h_w_ratio_max_int = this->armor_config.light_cfg.RATIO_W_H_MAX * 100;  //转 int

#endif  // !RELEASE

  private:
    // 候选装甲板
    CandidateArmor         armor;
    vector<CandidateArmor> armor_box;    // TODO：预设大小
    CandidateArmor         final_armor;  // 最终目标
    CandidateArmor         last_armor;   // 上一帧目标

    Mat armor_img;  // 装甲板筛选效果图

    // 高度差
    float light_y_diff;
    // 间距
    float light_distance;
    float light_height_max;
    float light_height_min;
    // 宽高比TODO 可能放去最优选择那边
    float armor_aspect_ratio;
    // 角度关系
    // 两灯条大小比例关系
    float light_size_ratio;

    // 双指针匹配
    int  ra;                           // 右灯条
    int  la;                           // 左灯条
    int  armor_index;                  // 装甲板索引号
    int  over_cnt;                     // 匹配失败计数器
    bool is_include_armor    = false;  // 是否跳过了一个灯条，标志位
    int  include_armor_index = 0;      // 记录跨越灯条索引号

    // 五个条件
    bool is_height_diff_catch    = false;
    bool is_light_distance_catch = false;
    bool is_similar_size         = false;
    bool is_light_angle_catch    = false;
    bool is_height_ratio         = false;

#ifndef RELEASE
    // trackbar
    int diff_light_y         = this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y * 10;
    int distance_multiple_th = this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH * 10;
    int distance_min_th      = this->armor_config.candidate_armor_cfg.DISTANCE_MIN_TH;
    int diff_angle_th_same   = this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_SAME * 10;
    int light_size_ratio_min = this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN * 10;
    int light_size_ratio_max = this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX * 10;

    int is_enable_diff_light_y         = 0;  //强制判断条件开关
    int is_enable_distance_multiple_th = 0;  //强制判断条件开关
    int is_enable_diff_angle_th        = 0;  //强制判断条件开关
    int is_enable_similar_size         = 0;  //强制判断条件开关
    int is_enable_height_ratio         = 0;  //强制判断条件开关
#endif                                       // !RELEASE

  private:
    // 最优装甲板
    float dist_to_center_min;  // 离中心点区域最小值
    float aspest_ratio_min;    // 斜率最小值

    vector<Point2f> target_2d;  //目标装甲板的序号排列

  private:
    // 标志位
    // 是否检测到装甲板
    bool is_last_data_catch = false;
    // 是否丢失目标
    bool is_Lost_target = true;
    // 丢失目标计数器
    int lose_target_cnt = 0;
    // 索敌范围
    int armor_range = 200;
    int change_buff_cnt = 10;

#ifdef DEBUG
  private:
    RM_FPS armor_fps;
    int    change_roi_flag = 0;
#endif  // DEBUG
};