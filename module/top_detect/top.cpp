#include "module/top_detect/top.hpp"
namespace top {

void Armor_Top::topStatusInitializ() {
  top_data_.status = STOP;
  top_data_.is_shooting = 0;
  top_data_.top_cycle = 0;
}

float Armor_Top::run_Top(cv::Mat &_src_img, float _data,
                         cv::Point _armor_center) {
  cv::namedWindow("top_trackbar");
  cv::createTrackbar("minimum_gap", "top_trackbar", &minimum_gap_, 100000,
                     NULL);
  cv::createTrackbar("minimum_error_", "top_trackbar", &minimum_error_, 1000,
                     NULL);
  cv::createTrackbar("minimum_min_cycle_count_gap", "top_trackbar",
                     &min_cycle_count_, 1000, NULL);
  cv::createTrackbar("max_cycle_count_", "top_trackbar", &max_cycle_count_,
                     1000, NULL);
  cv::createTrackbar("true_floor", "top_trackbar", &true_floor_, 1000, NULL);
  cv::createTrackbar("true_upper", "top_trackbar", &true_upper_, 1000, NULL);
  cv::imshow("top_trackbar", top_trackbar_);
  count_++;
  if (count_ == 1) {
    lost_data_ = _data;
  } else if (count_ > min_cycle_count_ && count_ < max_cycle_count_) {
    // 每帧差值
    int different = _data - lost_data_;
    data_different_ += different;
    // 差值和接近0
    if (fabs(data_different_) < minimum_error_ * 0.01) {
      judge_count_++;
      if (judge_count_ > true_upper_) {
        judge_count_ = true_upper_;
      }
    } else {
      judge_count_--;
      if (judge_count_ < 0) {
        judge_count_ = 0;
      }
    }
    if (judge_count_ < true_floor_) {
      status_ = STOP;
    } else {
      status_ = ISTOP;
    }
  } else {
    countInitializ();
    lost_data_ = 0;
  }
  return status_;
}

Armor_Top::Armor_Top() {}
Armor_Top::~Armor_Top() {}
}  // namespace top