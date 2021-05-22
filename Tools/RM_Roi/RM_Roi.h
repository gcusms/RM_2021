#pragma once

#include <opencv4/opencv2/opencv.hpp>

using namespace cv;

typedef struct Roi_Config
{
    // 参数
    int FIRST_LOSE_CNT_MAX;
    int SECOND_LOSE_CNT_MAX;
} Roi_Cfg;

class RM_ROI {
  public:
    // 构造函数
    RM_ROI(const Roi_Cfg& _roi_config);
    // 更新装甲板ROI
    void updateROI(Mat& _src, bool is_lost_target, short int cnt, const RotatedRect last_roi_rect);

    void resetParam();
    
    Point2f ROItl();

    Roi_Cfg& returnRoiConfig();

  public:
    // ROI的图像
    Mat roi_img;

  private:
    //   安全处理
    void safeTreatment(Point& _tl,int & _W,int& _H,const RotatedRect& _last_roi_rect,const int & _src_W,const int & _src_H);
    // 数据更新
    void inputParam(const RotatedRect& _last_roi_rect,
                    const Point2f&     _tl,
                    const int&         _width,
                    const int&         _height);

  private:
    Roi_Cfg roi_config;
    // 上一帧的装甲矩形
    RotatedRect last_armor_rect = RotatedRect();
    // roi的左上角点
    Point2f tl ;
    // 宽度
    int width = 0;
    // 高度
    int height=0;
};
