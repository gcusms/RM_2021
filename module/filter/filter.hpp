#ifndef RM_KALMANFILTER_H
#define RM_KALMANFILTER_H

#include <iostream>
using namespace std;
#include <opencv2/opencv.hpp>
#include <vector>
using namespace cv;
#include <queue>

#include "module/armor/rm_armor.hpp"
#include "rm_serial_port.hpp"

class RM_kalmanfilter {
 private:
  cv::KalmanFilter KF_;
  Mat measurement_matrix;
  cv::Mat filter_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);
  cv::Point lost_armor_center = cv::Point();
  int multiple_ = 10;
  int first_ignore_time_ = 5;
  int armor_threshold_max_ = 20;
  float p;
  float x = 640;
  float y = 320;

  typedef struct {
    const int QUEUE_NUM = 5 + 1;         // 队列大小+1
    float top_angle;                     // 上一刻陀螺仪数据
    float top_angle_;                    // 当前的陀螺仪数据
    float differ;                        // 前后时刻的差值
    int get_top_times = 0;               // 陀螺仪数据获取次数
    std::queue<float> difference_queue;  // difference_队列
  } top_diff_Queue;
  top_diff_Queue temp;
  top_diff_Queue *top_angle_differ = &temp;

 public:
  RM_kalmanfilter();
  ~RM_kalmanfilter();
  Point2f predict_point(Point2f _p);
  void reset();

  float use_RM_KF(float top, float _yaw_angle, cv::Point armor_center);
  // void use_RM_kf(top_diff_Queue *top_angle_differ, serial_port::SerialPort
  // serial_);

  float anti_range = 1.5;
};

#endif  // RM_KALMANFILTER_H