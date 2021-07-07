#ifndef TARGET_HPP_
#define TARGET_HPP_

#include "Fan_Armor/Fan_Armor.hpp"
#include "Fan_Blade/Fan_Blade.hpp"

namespace buff {
class Target {
public:
  // 构造函数 析构函数
  Target();
  ~Target();

  /**
   * @brief 输入参数
   * @param[in]  _fan_blade       外轮廓（扇叶）
   * @param[in]  _fan_armor       内轮廓（装甲板）
   */
  void inputParams(const FanBlade& _fan_blade, const FanArmor& _fan_armor);

  /**
   * @brief 更新装甲板的四个顶点编号
   * @param[in]  _img             输入绘制图像
   * @note 将各个顶点绘制在输入图像上
   */
  void updateVertex(cv::Mat& _img);

  /**
   * @brief 设置运转类型
   * @details 强制
   * @param[in]  _type            类型
   */
  void setType(const ObjectType& _type);

  /**
   * @brief 设置运转类型
   * @param[in]  _bin_img         输入二值图
   * @param[in]  _img             输入绘制图像
   * @todo 待添加新版roi
   */
  void setType(cv::Mat& _bin_img, cv::Mat& _img);

  /**
   * @brief 显示可打击的目标
   * @param[in]  _img             输入绘制图像
   * @note 装甲板为绿框，扇叶为黄框
   */
  void displayInactionTarget(cv::Mat& _img);

  /**
   * @brief 返回扇叶大轮廓
   * @return FanBlade
   */
  FanBlade Blade();

  /**
   * @brief 返回装甲板小轮廓
   * @return FanArmor
   */
  FanArmor Armor();

  /**
   * @brief 返回目标角度
   * @details 水平线右上为360°，右下为0°，顺时针增长
   * @return float
   */
  float Angle();

  /**
   * @brief 返回角度差
   * @return float
   */
  float diffAngle();

  /**
   * @brief 返回当前目标运转类型
   * @return ObjectType
   */
  ObjectType Type();

  /**
   * @brief 返回面积比例
   * @return double
   */
  double areaRatio();

  /**
   * @brief 返回装甲板排序后的顶点
   * @return vector<Point2f>
   * @todo 留做返回给pnp的接口，或者是留做返回给基类的接口
   */
  std::vector<cv::Point2f> vector2DPoint();

  /**
   * @brief 返回装甲板排序具体顶点
   * @param  _i               顶点编号
   * @return Point2f
   */
  cv::Point2f vector2DPoint(const int& _i);

  /**
   * @brief 返回装甲板高度点差
   * @return Point2f
   * @note 由于是点之间的差值，所以相减的顺序会影响变化的方向，要留意
   */
  cv::Point2f deltaPoint();

private:
  // 大轮廓
  FanBlade fan_blade_;
  // 小轮廓
  FanArmor fan_armor_;

  // 角度（可定坐标系）
  float angle_;
  // 大小轮廓角度差
  float diff_angle_;
  // 运转类型
  ObjectType type_;
  // 面积比例:小/大
  double area_ratio_;
  // 顶点容器
  std::vector<cv::Point2f> points_2d_;
  // 装甲板高度点差
  cv::Point2f delta_height_point_;
};

inline FanBlade Target::Blade()
{
  return this->fan_blade_;
}

inline FanArmor Target::Armor()
{
  return this->fan_armor_;
}

inline float Target::Angle()
{
  return this->angle_;
}

inline float Target::diffAngle()
{
  return this->diff_angle_;
}

inline ObjectType Target::Type()
{
  return this->type_;
}

inline double Target::areaRatio()
{
  return this->area_ratio_;
}

inline std::vector<cv::Point2f> Target::vector2DPoint()
{
  return this->points_2d_;
}

inline cv::Point2f Target::vector2DPoint(const int& _i)
{
  return this->points_2d_[_i];
}

inline cv::Point2f Target::deltaPoint()
{
  return this->points_2d_[0] - this->points_2d_[3];
}

}  // namespace buff

#endif  // !TARGET_HPP_
