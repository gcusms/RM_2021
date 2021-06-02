#ifndef _RM_LIGHT_HPP
#define _RM_LIGHT_HPP

#include <iostream>
#include <opencv2/opencv.hpp>

struct Light_Cfg {
  int light_edit = 0;
  //灯条宽高比范围
  int ratio_w_h_min = 1;
  int ratio_w_h_max = 6;
  //灯条角度范围
  int angle_min = 550;
  int angle_max = 1250;
  //灯条长度范围
  int perimeter_max = 0;
  int perimeter_min = 0;
};

struct Light_Data {
  // 宽度
  int width = 0;
  // 高度
  int height = 0;
  // 周长
  int perimeter = 0;
  // 角度
  float angle = 0.f;
  // 高宽比
  float aspect_ratio = 0.f;
  // 中心点
  cv::Point center;
};

class Armor_Light {
 private:
  Light_Cfg light_config_;
  cv::Mat light_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);

 public:
  void set_Light_Config(Light_Cfg _light_config);

  bool run_Find_Light(cv::Mat &_bin_img, std::vector<Light_Data> _light);
  void inputparam(cv::RotatedRect _box, Light_Data _light,
                  std::vector<cv::Point> _contours);
  bool whether_Light(Light_Data _light);
};

#endif  // !_RM_LIGHT_HPP