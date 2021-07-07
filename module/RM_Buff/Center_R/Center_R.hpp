#ifndef CENTER_R_HPP_
#define CENTER_R_HPP_

#include "Object/Object.hpp"

namespace buff {
class Center_R : public Object {
public:
  /**
   * @brief 输入参数
   * @param[in]  _contours        轮廓点集
   * @param[in]  _roi_img         圆形区域ROI
   */
  void inputParams(const std::vector<cv::Point>& _contours, const cv::Mat& _roi_img);

  /**
   * @brief 返回目标距离ROI图像中心点的距离
   * @return float
   */
  float centerDist();

private:
  // 中心距离
  float center_distance;
};

inline float Center_R::centerDist()
{
  return this->center_distance;
}

}

#endif  // !CENTER_R_HPP_