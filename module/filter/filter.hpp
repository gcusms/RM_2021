#ifndef RM_KALMANFILTER_H
#define RM_KALMANFILTER_H

#include <math.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>
#include <vector>

#include "rm_serial_port.hpp"
namespace kalman {
class RM_kalmanfilter {
 private:
  cv::KalmanFilter KF_;
  cv::Mat measurement_matrix;
  cv::Mat filter_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);
  cv::Point lost_armor_center;
  int multiple_ = 10;
  int first_ignore_time_ = 5;
  int armor_threshold_max_ = 20;
  float p;
  float x = 640;
  float y = 320;

  typedef struct {
    const int QUEUE_NUM = 5 + 1;  // 队列大小+1
    float top_angle;              // 上一刻陀螺仪数据
    float top_angle_;             // 当前的陀螺仪数据
    float differ;                 // 前后时刻的差值
    int get_top_times = 0;        // 陀螺仪数据获取次数
  } top_diff_Queue;
  top_diff_Queue temp;
  top_diff_Queue top_angle_differ;

 public:
  RM_kalmanfilter();
  ~RM_kalmanfilter();
  cv::Point2f predict_point(cv::Point2f _p);
  void reset();

  float use_RM_KF(float _top, float _yaw_angle, cv::Point _armor_center);
  // void use_RM_kf(top_diff_Queue *top_angle_differ, serial_port::SerialPort
  // serial_);
  float anti_range = 1.5;
};

}  // namespace kalman
#endif  // RM_KALMANFILTER_H