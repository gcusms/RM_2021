#include "Fan_Blade.hpp"

namespace buff {

void FanBlade::inputParams(const std::vector<cv::Point>& _contours)
{
  Object::inputParams(_contours);
}

FanBlade::FanBlade()
{
  this->length_ = 0;
}

FanBlade::~FanBlade() {}

void FanBlade::displayFanBlade(cv::Mat& _img)
{
  cv::Scalar color = cv::Scalar(0, 255, 255);

  cv::Point put_fan_blade = cv::Point(_img.cols - 100, 30);
  cv::putText(_img, " Fan Blade ", put_fan_blade, cv::FONT_HERSHEY_PLAIN, 1, color, 1, 8, false);

  for (int k = 0; k < 4; ++k) {
    cv::line(_img, this->vertex_[k], this->vertex_[(k + 1) % 4], color, 2);
  }
}
}  // namespace buff