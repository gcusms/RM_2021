#ifndef _RM_ARMOR_HPP_
#define _RM_ARMOR_HPP_

#include <cmath>

#include "rm_armor/image.hpp"
#include "rm_armor/light.hpp"

struct Armor_Cfg {
  int armor_edit = 0;

  int light_height_ratio_min = 7;
  int light_height_ratio_max = 15;

  int light_width_ratio_min = 7;
  int light_width_ratio_max = 50;

  int light_y_different = 10;
  int light_height_different = 10;
  int armor_angle_different = 80;

  int small_armor_aspect_min = 9;
  int armor_type_th = 29;
  int big_armor_aspect_max = 80;
};

struct Armor_Data {
  cv::RotatedRect armor_rect;  //装甲板旋转矩形

  float width = 0;         //装甲板宽度
  float height = 0;        //装甲板高度
  float aspect_ratio = 0;  //装甲板宽高比
  float tan_angle = 0;     //装甲板角度

  float light_height_aspect = 0.f;
  float light_width_aspect = 0.f;

  Light_Data left_light;
  Light_Data right_light;

  float min_light_h = 0;  //最小灯条高度
  float max_light_h = 0;  //最大灯条高度

  int depth = 0;            //装甲板深度
  int priority = 0;         //优先级 多装甲板情况
  int distinguish = 0;      ///大小装甲板 小0 大1
  int position = 0;         //装甲板在车的左(-1)右(1)位置
  int distance_center = 0;  // 距离图像中心点距离
};

struct Final_Armor {};

class Rm_Armor : public Armor_Image, public Armor_Light {
 private:
  std::vector<Light_Data> light_;  // 灯条数据
  std::vector<Armor_Data> armor_;  // 装甲板数据
  Armor_Cfg armor_config_;
  cv::Mat armor_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);
  cv::Mat draw_img_;

 public:
  bool run_Armor(cv::Mat &_src_img, const int _my_color);

  void final_Armor();

  bool matching_Armor();
  bool light_Judge(Armor_Data _armor_data);

  float distance(cv::Point _point1, cv::Point _point2);

  Rm_Armor();
  Rm_Armor(std::string _armor_config);
  ~Rm_Armor();
};

#endif  // !_RM_ARMOR_HPP_