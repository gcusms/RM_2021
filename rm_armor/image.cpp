#include "rm_armor/image.hpp"

void Armor_Image::set_Image_Config(Image_Cfg _Image_Config) {
  image_config_ = _Image_Config;
  std::cout << "预处理参数初始化成功" << std::endl;
  std::cout << "💚💚💚💚💚💚💚💚💚💚💚💚" << std::endl;
}
/**
 * @brief 预处理运行函数
 *
 * @param _src_img 原图像
 * @param _my_color 自身颜色
 * @param image_config_ 固定参数
 * @return cv::Mat
 */
cv::Mat Armor_Image::run_Image(cv::Mat &_src_img, const int _my_color) {
  switch (image_config_.method) {
    case 0:
      return this->fuse_Image(this->gray_Pretreat(_src_img, _my_color),
                              this->bgr_Pretreat(_src_img, _my_color));
      break;
    default:
      return this->fuse_Image(this->gray_Pretreat(_src_img, _my_color),
                              this->hsv_Pretreat(_src_img, _my_color));
      break;
  }
}

/**
 * @brief 合并图像
 *
 * @param _bin_gray_img 灰度二值图
 * @param _bin_color_img 颜色二值图
 * @return cv::Mat 合并二值图
 */
cv::Mat Armor_Image::fuse_Image(cv::Mat _bin_gray_img, cv::Mat _bin_color_img) {
  cv::bitwise_or(_bin_color_img, _bin_gray_img, _bin_color_img);
  morphologyEx(_bin_color_img, _bin_color_img, cv::MORPH_DILATE, this->ele_);
  return _bin_color_img;
}

/**
 * @brief 灰度图处理
 *
 * @param _src_img 原图像
 * @param image_config_ 预处理参数
 * @param _my_color 自身颜色
 * @param _edit 调参开启
 * @return cv::Mat 返回二值图
 */
cv::Mat Armor_Image::gray_Pretreat(cv::Mat &_src_img, const int _my_color) {
  static cv::Mat bin_gray_img, gray_img;
  cv::cvtColor(_src_img, gray_img, cv::COLOR_BGR2GRAY);
  switch (_my_color) {
    case 0:
      if (image_config_.gray_edit) {
        cv::namedWindow("gray_trackbar");
        cv::createTrackbar("gray_th", "gray_trackbar",
                           &image_config_.blue_armor_gray_th, 255, NULL);
        cv::imshow("gray_trackbar", this->gray_trackbar_);
      }
      cv::threshold(gray_img, bin_gray_img, image_config_.blue_armor_gray_th,
                    255, cv::THRESH_BINARY);
      break;
    default:
      if (image_config_.gray_edit) {
        cv::namedWindow("gray_trackbar");
        cv::createTrackbar("gray_th", "gray_trackbar",
                           &image_config_.red_armor_gray_th, 255, NULL);
        cv::imshow("gray_trackbar", this->gray_trackbar_);
      }
      cv::threshold(gray_img, bin_gray_img, image_config_.red_armor_gray_th,
                    255, cv::THRESH_BINARY);
      break;
  }
  return bin_gray_img;
}

/**
 * @brief bgr
 *
 * @param _src_img 原图像
 * @param _my_color 自身颜色
 * @param image_config_ 预处理参数
 * @param _edit 调参开启
 * @return cv::Mat 返回二值图
 */
cv::Mat Armor_Image::bgr_Pretreat(cv::Mat &_src_img, const int _my_color) {
  static std::vector<cv::Mat> _split;
  cv::split(_src_img, _split);
  static cv::Mat bin_color_img;
  switch (_my_color) {
    case 0:
      cv::subtract(_split[2], _split[0], bin_color_img);  // b - r
      if (image_config_.color_edit) {
        cv::namedWindow("color_trackbar");
        cv::createTrackbar("blue_color_th", "color_trackbar",
                           &image_config_.blue_armor_color_th, 255, NULL);
        cv::imshow("color_trackbar", this->bgr_trackbar_);
      }
      cv::threshold(bin_color_img, bin_color_img,
                    image_config_.blue_armor_color_th, 255, cv::THRESH_BINARY);
      break;
    default:
      cv::subtract(_split[0], _split[2], bin_color_img);  // r - b
      if (image_config_.color_edit) {
        cv::namedWindow("color_trackbar");
        cv::createTrackbar("red_color_th", "color_trackbar",
                           &image_config_.red_armor_color_th, 255, NULL);
        cv::imshow("color_trackbar", this->bgr_trackbar_);
      }
      cv::threshold(bin_color_img, bin_color_img,
                    image_config_.red_armor_color_th, 255, cv::THRESH_BINARY);
      break;
  }
  return bin_color_img;
}

/**
 * @brief hsv预处理
 *
 * @param _src_img 原图像
 * @param image_config_ 预处理参数
 * @param _my_color 自身颜色
 * @param _edit 调参开始
 * @return cv::Mat
 */
cv::Mat Armor_Image::hsv_Pretreat(cv::Mat &_src_img, const int _my_color) {
  static cv::Mat hsv_img, bin_color_img;
  cv::cvtColor(_src_img, hsv_img, cv::COLOR_BGR2HSV_FULL);
  switch (_my_color) {
    case 0:

      if (image_config_.color_edit) {
        cv::namedWindow("hsv_trackbar");
        cv::createTrackbar("blue_h_min:", "trackbar", &image_config_.h_blue_min,
                           255, nullptr);
        cv::createTrackbar("blue_h_max:", "trackbar", &image_config_.h_blue_max,
                           255, nullptr);
        cv::createTrackbar("blue_s_min:", "trackbar", &image_config_.s_blue_min,
                           255, nullptr);
        cv::createTrackbar("blue_s_max:", "trackbar", &image_config_.s_blue_max,
                           255, nullptr);
        cv::createTrackbar("blue_v_min:", "trackbar", &image_config_.v_blue_min,
                           255, nullptr);
        cv::createTrackbar("blue_v_max:", "trackbar", &image_config_.v_red_max,
                           255, nullptr);
        cv::imshow("hsv_trackbar", this->hsv_trackbar_);
      }

      cv::inRange(hsv_img,
                  cv::Scalar(image_config_.h_blue_min, image_config_.s_blue_min,
                             image_config_.v_blue_min),
                  cv::Scalar(image_config_.h_blue_max, image_config_.s_blue_max,
                             image_config_.v_blue_max),
                  bin_color_img);

      break;
    default:

      if (image_config_.color_edit) {
        cv::namedWindow("hsv_trackbar");
        cv::createTrackbar("red_h_min:", "trackbar", &image_config_.h_red_min,
                           255, nullptr);
        cv::createTrackbar("red_h_max:", "trackbar", &image_config_.h_red_max,
                           255, nullptr);
        cv::createTrackbar("red_s_min:", "trackbar", &image_config_.s_red_min,
                           255, nullptr);
        cv::createTrackbar("red_s_max:", "trackbar", &image_config_.s_red_max,
                           255, nullptr);
        cv::createTrackbar("red_v_min:", "trackbar", &image_config_.v_red_min,
                           255, nullptr);
        cv::createTrackbar("red_v_max:", "trackbar", &image_config_.v_red_max,
                           255, nullptr);
        cv::imshow("hsv_trackbar", this->hsv_trackbar_);
      }

      cv::inRange(hsv_img,
                  cv::Scalar(image_config_.h_red_min, image_config_.s_red_min,
                             image_config_.v_red_min),
                  cv::Scalar(image_config_.h_red_max, image_config_.s_red_max,
                             image_config_.v_red_max),
                  bin_color_img);

      break;
  }
  return bin_color_img;
}