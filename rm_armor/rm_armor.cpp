#include "rm_armor/rm_armor.hpp"

Rm_Armor::Rm_Armor() {}
Rm_Armor::~Rm_Armor() {}

Rm_Armor::Rm_Armor(std::string _armor_config) {
  cv::FileStorage fs_armor(_armor_config, cv::FileStorage::READ);
  //预处理初始化
  Image_Cfg image_config;
  //预处理调参开关
  fs_armor["GRAY_EDIT"] >> image_config.gray_edit;
  fs_armor["COLOR_EDIT"] >> image_config.color_edit;
  fs_armor["METHOD"] >> image_config.method;
  fs_armor["BLUE_ARMOR_GRAY_TH"] >> image_config.blue_armor_gray_th;
  fs_armor["RED_ARMOR_GRAY_TH"] >> image_config.red_armor_gray_th;
  if (image_config.method == 0) {
    fs_armor["RED_ARMOR_COLOR_TH"] >> image_config.red_armor_color_th;
    fs_armor["BLUE_ARMOR_COLOR_TH"] >> image_config.blue_armor_color_th;
  } else {
    fs_armor["H_RED_MIN"] >> image_config.h_red_min;
    fs_armor["H_RED_MAX"] >> image_config.h_red_max;
    fs_armor["S_RED_MIN"] >> image_config.s_red_min;
    fs_armor["S_RED_MAX"] >> image_config.s_red_max;
    fs_armor["V_RED_MIN"] >> image_config.v_red_min;
    fs_armor["V_RED_MAX"] >> image_config.v_red_max;

    fs_armor["H_BLUE_MIN"] >> image_config.h_blue_min;
    fs_armor["H_BLUE_MAX"] >> image_config.h_blue_max;
    fs_armor["S_BLUE_MIN"] >> image_config.s_blue_min;
    fs_armor["S_BLUE_MAX"] >> image_config.s_blue_max;
    fs_armor["V_BLUE_MIN"] >> image_config.v_blue_min;
    fs_armor["V_BLUE_MAX"] >> image_config.v_blue_max;
  }
  this->set_Image_Config(image_config);

  //灯条匹配参数初始化
  Light_Cfg light_config;
  fs_armor["LIGHT_EDIT"] >> light_config.light_edit;
  fs_armor["LIGHT_RATIO_W_H_MIN"] >> light_config.ratio_w_h_min;
  fs_armor["LIGHT_RATIO_W_H_MAX"] >> light_config.ratio_w_h_max;

  fs_armor["LIGHT_ANGLE_MIN"] >> light_config.angle_min;
  fs_armor["LIGHT_ANGLE_MAX"] >> light_config.angle_max;

  fs_armor["LIGHT_PERIMETER_MIN"] >> light_config.perimeter_min;
  fs_armor["LIGHT_PERIMETER_MAX"] >> light_config.perimeter_max;
  this->set_Light_Config(light_config);

  //装甲匹配参数初始化
  fs_armor["ARMOR_HEIGHT_RATIO_MIN"] >> armor_config_.light_height_ratio_min;
  fs_armor["ARMOR_HEIGHT_RATIO_MAX"] >> armor_config_.light_height_ratio_max;

  fs_armor["ARMOR_WIDTH_RATIO_MIN"] >> armor_config_.light_width_ratio_min;
  fs_armor["ARMOR_WIDTH_RATIO_MAX"] >> armor_config_.light_width_ratio_max;

  fs_armor["ARMOR_Y_DIFFERENT"] >> armor_config_.light_y_different;
  fs_armor["ARMOR_HEIGHT_DIFFERENT"] >> armor_config_.light_height_different;
  fs_armor["ARMOR_ANGLE_DIFFERENT"] >> armor_config_.armor_angle_different;

  fs_armor["ARMOR_SMALL_ASPECT_MIN"] >> armor_config_.small_armor_aspect_min;
  fs_armor["ARMOR_TYPE_TH"] >> armor_config_.armor_type_th;
  fs_armor["ARMOR_BIG_ASPECT_MAX"] >> armor_config_.big_armor_aspect_max;

  std::cout << "装甲板参数初始化成功" << std::endl;
  std::cout << "💚💚💚💚💚💚💚💚💚💚💚💚" << std::endl;
}
/**
 * @brief 寻找armor 运行函数
 *
 * @param _src_img 原图像
 * @param _my_color 自身颜色
 * @return true 找到
 * @return false 没找到
 */
bool Rm_Armor::run_Armor(cv::Mat &_src_img, const int _my_color) {
  draw_img_ = _src_img.clone();
  cv::Mat bin_img = this->run_Image(_src_img, _my_color);
  if (this->run_Find_Light(bin_img, this->light_)) {
    if (this->matching_Armor()) {
      final_Armor();
    }
  }
  if (armor_config_.armor_edit == 1) {
    cv::imshow("armor", this->draw_img_);
  }
}

/**
 * @brief 判断大小
 *
 * @param _a 结构体一
 * @param _b 结构体二
 * @return true
 * @return false
 */
bool comparison(Armor_Data _a, Armor_Data _b) {
  return _a.distance_center < _b.distance_center;  // ‘<’升序 | ‘>’降序
}

/**
 * @brief 装甲板排序(离装甲板最近)
 *
 * @param armor_ 装甲板数据
 */
void Rm_Armor::final_Armor() {
  if (armor_.size() == 1) {
    std::cout << "只有一个装甲板" << std::endl;
  }
  std::cout << "有多个装甲板" << std::endl;
  std::sort(armor_.begin(), armor_.end(), comparison);
  if (armor_config_.armor_type_th == 1) {
    cv::rectangle(draw_img_, armor_[0].armor_rect.boundingRect(),
                  cv::Scalar(0, 255, 0), 3, 8);
  }
}

/**
 * @brief 灯条拟合装甲板
 * @return true 是装甲板
 * @return false other
 */
bool Rm_Armor::matching_Armor() {
  static Armor_Data armor_data;
  for (size_t i = 0; i < light_.size(); ++i) {
    for (size_t j = i + 1; j < light_.size(); ++j) {
      // 区分左右灯条
      int light_left = 0, light_right = 0;
      if (light_[i].center.x > light_[j].center.x) {
        light_left = j;
        light_right = i;
      } else {
        light_left = i;
        light_right = j;
      }

      //保存左右灯条数据
      armor_data.left_light = light_[light_left];
      armor_data.right_light = light_[light_right];

      // 装甲板斜率
      float error_angle = std::atan(
          (light_[light_right].center.y - light_[light_left].center.y) /
          (light_[light_right].center.x - light_[light_left].center.x));

      if (error_angle < 10.f) {
        armor_data.tan_angle = error_angle * 180 / CV_PI;
        if (light_Judge(armor_data)) {
          if (armor_config_.armor_edit == 1) {
            cv::rectangle(draw_img_, armor_data.armor_rect.boundingRect(),
                          cv::Scalar(0, 255, 0), 3, 8);
          }
          armor_.push_back(armor_data);
        }
      }
    }
  }

  if (armor_.size() > 0) {
    return true;
  }
  std::cout << "装甲板匹配失败" << std::endl;
  return false;
}

/**
 * @brief 拟合装甲板
 *
 * @param _armor_data 装甲板数据记录
 * @return true 有
 * @return false other
 */
bool Rm_Armor::light_Judge(Armor_Data _armor_data) {
  _armor_data.light_height_aspect =
      _armor_data.left_light.height / _armor_data.right_light.height;
  _armor_data.light_width_aspect =
      _armor_data.left_light.width / _armor_data.right_light.width;

  if (armor_config_.armor_edit == 1) {
    cv::namedWindow("armor_trackbar");
    cv::createTrackbar("light_height_aspect_min", "armor_trackbar",
                       &armor_config_.light_height_ratio_min, 100);
    cv::createTrackbar("light_height_aspect_max", "armor_trackbar",
                       &armor_config_.light_height_ratio_max, 100);
    cv::createTrackbar("light_width_aspect_min", "armor_trackbar",
                       &armor_config_.light_width_ratio_min, 100);
    cv::createTrackbar("light_width_aspect_min", "armor_trackbar",
                       &armor_config_.light_width_ratio_min, 100);

    cv::createTrackbar("light_y_different", "armor_trackbar",
                       &armor_config_.light_y_different, 100);
    cv::createTrackbar("light_height_different", "armor_trackbar",
                       &armor_config_.light_height_different, 100);

    cv::createTrackbar("armor_angle_different", "armor_trackbar",
                       &armor_config_.armor_angle_different, 100);

    cv::createTrackbar("small_armor_aspect_min", "armor_trackbar",
                       &armor_config_.small_armor_aspect_min, 100);
    cv::createTrackbar("armor_type_th", "armor_trackbar",
                       &armor_config_.armor_type_th, 100);
    cv::createTrackbar("big_armor_aspect_max", "armor_trackbar",
                       &armor_config_.big_armor_aspect_max, 100);
    cv::imshow("armor_trackbar", armor_trackbar_);
  }

  if (_armor_data.light_height_aspect <
          armor_config_.light_height_ratio_max * 0.1 &&
      _armor_data.light_height_aspect >
          armor_config_.light_height_ratio_min * 0.1 &&
      _armor_data.light_width_aspect <
          armor_config_.light_width_ratio_max * 0.1 &&
      _armor_data.light_width_aspect >
          armor_config_.light_height_ratio_min * 0.1) {
    _armor_data.height =
        (_armor_data.left_light.height + _armor_data.right_light.height) / 2;

    // 两个灯条高度差不大
    if (fabs(_armor_data.left_light.center.y -
             _armor_data.right_light.center.y) <
        _armor_data.height * (armor_config_.light_y_different) * 0.1) {
      if (fabs(_armor_data.left_light.height - _armor_data.right_light.height) <
          (_armor_data.height * armor_config_.light_height_different) * 0.1) {
        _armor_data.width = distance(_armor_data.left_light.center,
                                     _armor_data.right_light.center);
        _armor_data.aspect_ratio =
            _armor_data.width / _armor_data.height;  //装甲板长宽比
        //两侧灯条角度差
        if (fabs(_armor_data.left_light.angle - _armor_data.right_light.angle) <
            armor_config_.armor_angle_different * 0.1) {
          cv::RotatedRect rects = cv::RotatedRect(
              (_armor_data.left_light.center + _armor_data.right_light.center) /
                  2,
              cv::Size(_armor_data.width, _armor_data.height),
              _armor_data.tan_angle);
          _armor_data.armor_rect = rects;  //储存装甲板旋转矩形
          _armor_data.distance_center =
              distance(_armor_data.armor_rect.center,
                       cv::Point(this->draw_img_.cols, this->draw_img_.rows));
          if (_armor_data.aspect_ratio >
                  armor_config_.small_armor_aspect_min * 0.1 &&
              _armor_data.aspect_ratio < armor_config_.armor_type_th * 0.1) {
            _armor_data.distinguish = 0;  // 小装甲板
            return true;
          } else if (_armor_data.aspect_ratio >
                         armor_config_.armor_type_th * 0.1 &&
                     _armor_data.aspect_ratio <
                         armor_config_.big_armor_aspect_max * 0.1) {
            _armor_data.distinguish = 1;  //大装甲板
            return true;
          }
        }
      }
    }
  }
  return false;
}

/**
 * @brief 求两点之间的距离
 *
 * @param _point1 点A
 * @param _point2 点B
 * @return double 两点之间的距离
 */
float Rm_Armor::distance(cv::Point _point1, cv::Point _point2) {
  return sqrt((_point1.x - _point2.x) * (_point1.x - _point2.x) +
              (_point1.y - _point2.y) * (_point1.y - _point2.y));
}