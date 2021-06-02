#include "rm_armor/light.hpp"

void Armor_Light::set_Light_Config(Light_Cfg light_config_) {
  light_config_ = light_config_;
  std::cout << "灯条参数初始化成功" << std::endl;
  std::cout << "💚💚💚💚💚💚💚💚💚💚💚💚" << std::endl;
}

/**
 * @brief
 *
 * @param _bin_img 二值图传入
 * @param _light 数组输出灯条数据
 * @param light_config_ 判断灯条参数
 * @return true 找到一个以上灯条
 * @return false other
 */
bool Armor_Light::run_Find_Light(cv::Mat &_bin_img,
                                 std::vector<Light_Data> _light) {
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(_bin_img, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_NONE);

  if (contours.size() < 2) {
    std::cout << "轮廓数量少于2" << std::endl;
    std::cout << "❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌" << std::endl;
    return false;
  }

  static cv::RotatedRect box;
  static Light_Data light;

  for (size_t i = 0; i < contours.size(); ++i) {
    box = cv::fitEllipse(contours[i]);

    inputparam(box, light, contours[i]);

    if (whether_Light(light)) {
      _light.push_back(light);
    }
  }

  if (_light.size() > 1) {
    std::cout << "灯条数量正常" << std::endl;
    return true;
  }
  std::cout << "灯条数量少于2退出" << std::endl;
  std::cout << "❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌" << std::endl;
  return false;
}

/**
 * @brief 判断是否为灯条
 *
 * @param _light 灯条参数
 * @param light_config_ 灯条判断参数
 * @return true
 * @return false
 */
bool Armor_Light::whether_Light(Light_Data _light) {
  if (light_config_.light_edit == 1) {
    cv::namedWindow("light_trackbar");
    cv::createTrackbar("light_angle_min", "light_trackbar",
                       &light_config_.angle_min, 3600);
    cv::createTrackbar("light_angle_max", "light_trackbar",
                       &light_config_.angle_max, 3600);

    cv::createTrackbar("light_ratio_w_h_min", "light_trackbar",
                       &light_config_.ratio_w_h_min, 1000);
    cv::createTrackbar("light_ratio_w_h_max", "light_trackbar",
                       &light_config_.ratio_w_h_max, 1000);

    cv::createTrackbar("light_perimeter_min", "light_trackbar",
                       &light_config_.perimeter_min, 100000);
    cv::createTrackbar("light_perimeter_max", "light_trackbar",
                       &light_config_.perimeter_max, 100000);
  }

  if (_light.angle <= light_config_.angle_max * 0.1 &&
      _light.angle >= light_config_.angle_min * 0.1) {
    std::cout << "light_angle = " << _light.angle << std::endl;
    if (_light.aspect_ratio >= light_config_.ratio_w_h_min * 0.1 &&
        _light.aspect_ratio <= light_config_.ratio_w_h_max * 0.1) {
      std::cout << "light_aspect_ratio = " << _light.aspect_ratio << std::endl;
      if (_light.perimeter <= light_config_.perimeter_max * 0.1 &&
          _light.perimeter >= light_config_.perimeter_min * 0.1) {
        std::cout << "light_perimeter = " << _light.perimeter << std::endl;
        return true;
      }
    }
  }
  return false;
}

/**
 * @brief 处理灯条参数
 *
 * @param _box 旋转矩形
 * @param _light 灯条数据保存
 * @param _contours 点集
 */
void Armor_Light::inputparam(cv::RotatedRect _box, Light_Data _light,
                             std::vector<cv::Point> _contours) {
  _light.width = MIN(_box.size.width, _box.size.height);
  _light.height = MAX(_box.size.width, _box.size.height);
  _light.aspect_ratio = _light.width / _light.height;
  if (_box.angle >= 0.f && _box.angle <= 90.f) {
    _light.angle = _box.angle + 90.f;
  } else if (_box.angle > 90.f && _box.angle <= 180.f) {
    _light.angle = _box.angle - 90.f;
  }
  _light.perimeter = cv::arcLength(_contours, true);
  _light.center = _box.center;
}