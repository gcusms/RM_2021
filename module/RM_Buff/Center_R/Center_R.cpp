#include "Center_R.hpp"

namespace buff {
void Center_R::inputParams(const std::vector<cv::Point>& _contours, const cv::Mat& _roi_img)
{
  Object::inputParams(_contours);

  // 中心距离
  this->center_distance =
      centerDistance(this->rect_.center, cv::Point(_roi_img.cols * 0.5, _roi_img.rows * 0.5));
}
}  // namespace buff