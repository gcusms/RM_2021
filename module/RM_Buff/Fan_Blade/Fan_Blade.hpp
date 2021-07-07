#ifndef FAN_BLADE_HPP_
#define FAN_BLADE_HPP_

#include "Object/Object.hpp"

namespace buff {

class FanBlade : public Object {
public:
  FanBlade();
  ~FanBlade();

  /**
   * @brief 输入外轮廓参数
   * @param[in]  _contours        外轮廓点集
   */
  void inputParams(const std::vector<cv::Point>& _contours);

  /**
   * @brief 显示外轮廓
   * @param[out]  _img         输出图像
   * @note 黄色
   * @note 图例显示在右侧
   */
  void displayFanBlade(cv::Mat& _img);

  /**
   * @brief 返回周长
   * @return int
   */
  int Length();

private:
  // 周长
  int length_;
};

inline int FanBlade::Length()
{
  return this->length_;
}

}  // namespace buff

#endif  // !FAN_BLADE_HPP_