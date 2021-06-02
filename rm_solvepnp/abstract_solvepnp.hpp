// #ifndef RM_SOLVEPNP_ABSTRACT_HPP_
// #define RM_SOLVEPNP_ABSTRACT_HPP_

#include <iostream>
#include <opencv2/opencv.hpp>

struct Solvepnp_Cfg {
  int company = 1;

  int draw_xyz = 0;

  double ptz_camera_x = 0.0;
  double ptz_camera_y = 0.0;
  double ptz_camera_z = 0.0;

  float barrel_ptz_offset_x = 0.0;
  float barrel_ptz_offset_y = 0.0;

  float offset_armor_pitch = 0.0;
  float offset_armor_yaw = 0.0;
};

/**
 * @brief 装甲板实际长度单位（mm）
 *
 */
enum ARMOR {
  //小装甲板
  SMALL_ARMOR_HEIGHT = 60,
  SMALL_ARMOR_WIDTH = 140,
  //大装甲板
  BIG_ARMOR_WIDTH = 245,
  BIG_ARMOR_HEIGHT = 60,
  //灯条
  LIGHT_SIZE_W = 10,
  LIGHT_SIZE_H = 55,
  //大神符
  BUFF_ARMOR_WIDTH = 250,
  BUFF_ARMOR_HEIGHT = 65

};

class Abstract_Solvepnp_ {
 private:
  cv::Mat pnp_config_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);

 public:
  std::vector<cv::Point3f> initialize_3d_Points(int _armor_type);
  std::vector<cv::Point3f> initialize_3d_Points(int _width, int _heigth);

  std::vector<cv::Point2f> initialize_2d_Points(cv::RotatedRect _rect);
  std::vector<cv::Point2f> initialize_2d_Points(cv::Rect _rect);

  cv::RotatedRect rect_Change_Rotatedrect(cv::Rect _rect);

  cv::Mat camera_Ptz(cv::Mat &_t, double _ptz_camera_x, double _ptz_camera_y,
                     double _ptz_camera_z);

  void draw_Coordinate(cv::Mat &_draw_img, cv::Mat &_rvec, cv::Mat &_tvec,
                       cv::Mat &_cameraMatrix, cv::Mat &_distcoeffs);

  float get_Pitch(float _dist, float _tvec_y, float _ballet_speed,
                  const int _company = 1);

  cv::Point3f get_Angle(const cv::Mat &_pos_in_ptz, const int _bullet_speed,
                        const int _company, const float _barrel_ptz_offset_x,
                        const float _barrel_ptz_offset_y);
};
// #endif  // !