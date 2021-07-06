#ifndef FAN_ARMOR_HPP_
#define FAN_ARMOR_HPP_

#include "Object/Object.hpp"

namespace buff {
class FanArmor : public Object {
public:
  FanArmor();
  ~FanArmor();

  /**
   * @brief 输入参数
   * @param[in]  _contours        输入轮廓点集
   */
  void inputParams(const std::vector<cv::Point>& _contours);

  /**
   * @brief 显示内轮廓
   * @param[out]  _img     输出图像
   * @note 绿色
   * @note 图例显示在右侧
   */
  void displayFanArmor(cv::Mat& _img);

private:
  /* anything else */
};

}  // namespace buff

#endif  // !FAN_ARMOR_HPP_