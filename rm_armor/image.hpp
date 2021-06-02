#ifndef _RM_IMAGE_HPP_
#define _RM_IMAGE_HPP_

#include <iostream>
#include <opencv2/opencv.hpp>

struct Image_Cfg {
  // BGR
  int red_armor_gray_th;
  int red_armor_color_th;
  int blue_armor_gray_th;
  int blue_armor_color_th;
  // HSV-red
  int h_red_max;
  int h_red_min;
  int s_red_max;
  int s_red_min;
  int v_red_max;
  int v_red_min;
  // HSV-blue
  int h_blue_max;
  int h_blue_min;
  int s_blue_max;
  int s_blue_min;
  int v_blue_max;
  int v_blue_min;

  int gray_edit = 0;
  int color_edit = 0;
  int method = 0;
};

class Armor_Image {
 private:
  cv::Mat gray_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);
  cv::Mat bgr_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);
  cv::Mat hsv_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);
  cv::Mat ele_ = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

  Image_Cfg image_config_;

 public:
  void set_Image_Config(Image_Cfg _Image_Config);

  cv::Mat run_Image(cv::Mat &_src_img, const int _my_color);

  cv::Mat bgr_Pretreat(cv::Mat &_src_img, const int _my_color);

  cv::Mat hsv_Pretreat(cv::Mat &_src_img, const int _my_color);

  cv::Mat gray_Pretreat(cv::Mat &_src_img, const int _my_color);

  cv::Mat fuse_Image(cv::Mat _bin_gray_img, cv::Mat _bin_color_img);
};

#endif  // !_RM_IMAGE_HPP_