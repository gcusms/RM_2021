#include "filter.hpp"

RM_kalmanfilter::RM_kalmanfilter() : KF_(4, 2) {
  measurement_matrix = Mat::zeros(2, 1, CV_32F);
  KF_.transitionMatrix =
      (Mat_<float>(4, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

  setIdentity(KF_.measurementMatrix, Scalar::all(1));
  setIdentity(KF_.processNoiseCov, Scalar::all(1e-3));  //
  setIdentity(KF_.measurementNoiseCov,
              Scalar::all(1e-2));  //测量协方差矩阵R，数值越大回归越慢
  setIdentity(KF_.errorCovPost, Scalar::all(1));

  KF_.statePost = (Mat_<float>(4, 1) << x, y, 0, 0);  //初始值
}

RM_kalmanfilter::~RM_kalmanfilter() {}

Point2f RM_kalmanfilter::predict_point(Point2f _p) {
  // p = _p;
  Mat prediction = KF_.predict();
  Point2f predict_pt = Point2f(prediction.at<float>(0), 0);

  measurement_matrix.at<float>(0, 0) = _p.x;

  KF_.correct(measurement_matrix);
  return predict_pt;
}

void RM_kalmanfilter::reset() { measurement_matrix = Mat::zeros(2, 1, CV_32F); }

//
float RM_kalmanfilter::use_RM_KF(float _top, float _yaw_angle,
                                 cv::Point _armor_center) {
  float armor_vary = armor::Distance(lost_armor_center, _armor_center);
  lost_armor_center = _armor_center;
  cv::namedWindow("filter_trackbar");
  cv::createTrackbar("multiple_", "filter_trackbar", &multiple_, 1000, NULL);
  cv::createTrackbar("first_ignore_time_", "filter_trackbar",
                     &first_ignore_time_, 100, NULL);
  cv::createTrackbar("armor_threshold_max_", "filter_trackbar", &multiple_, 100,
                     NULL);
  imshow("filter_trackbar", filter_trackbar_);
  top_angle_differ->top_angle_ = _top;

  // 第一次获取的陀螺仪数据时, 对上一时刻(不存在)的陀螺仪数据的假设
  if (top_angle_differ->get_top_times == 0) {
    top_angle_differ->top_angle = 0;
  }

  std::cout << "top: " << _top << std::endl;

  top_angle_differ->get_top_times++;  // 自动计数 -> 获得陀螺仪数据的次数

  top_angle_differ->differ =
      top_angle_differ->top_angle_ - top_angle_differ->top_angle;

  top_angle_differ->top_angle =
      top_angle_differ
          ->top_angle_;  // 这一时刻的陀螺仪数据 更新为 上一时刻的陀螺仪数据

  std::cout << "top_angle_differ " << top_angle_differ->differ << std::endl;

  Point2f top_differ = Point2f(top_angle_differ->differ * multiple_, 0);
  if (top_angle_differ->get_top_times > first_ignore_time_ &&
      armor_vary < armor_threshold_max_) {
    return _yaw_angle + predict_point(top_differ).x;
  } else {
    return _yaw_angle;  // top_angle_differ->differ
  }
}