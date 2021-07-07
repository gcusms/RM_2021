#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include <opencv4/opencv2/opencv.hpp>
#include <vector>

namespace buff {
// 扇叶状态
enum ObjectType { UNKNOW, INACTION, ACTION };

float centerDistance(const cv::Point& p1, const cv::Point& p2);

int getRectIntensity(const cv::Mat& frame, cv::Rect rect);

//目标类（基类）
class Object {
public:
  /**
   * @brief Construct a new Object object
   */
  Object();

  /**
   * @brief Destroy the Object object
   */
  ~Object();

  /**
   * @brief 返回矩形
   * @return RotatedRect
   */
  cv::RotatedRect Rect();

  /**
   * @brief 返回宽度
   * @return float
   */
  float Width();

  /**
   * @brief 返回高度
   * @return float
   */
  float Height();

  /**
   * @brief 返回索引号
   * @return size_t
   */
  size_t Index();

  /**
   * @brief 返回角度
   * @return float
   */
  float Angle();

  /**
   * @brief 返回宽高比
   * @return float
   */
  float aspectRatio();

  /**
   * @brief 返回轮廓面积
   * @return float
   */
  float Area();

  /**
   * @brief 返回顶点点集
   * @return Point2f
   */
  cv::Point2f* Vertex();

  /**
   * @brief 返回特定顶点
   * @param  _i               顶点编号
   * @return Point2f
   */
  cv::Point2f Vertex(const int& _i);

protected:
  /**
   * @brief 输入参数
   * @param[in]  _contours        轮廓点集
   */
  void inputParams(const std::vector<cv::Point>& _contours);

protected:
  // 旋转矩形
  cv::RotatedRect rect_;
  // 宽
  float width_;
  // 高
  float height_;
  // 索引号
  size_t index_;
  // 角度
  float angle_;
  // 宽高比
  float aspect_ratio_;
  // 轮廓面积
  float area_;
  // 顶点点集
  cv::Point2f vertex_[4];
};

inline cv::RotatedRect Object::Rect()
{
  return this->rect_;
}

inline float Object::Width()
{
  return this->width_;
}

inline float Object::Height()
{
  return this->height_;
}

inline size_t Object::Index()
{
  return this->index_;
}

inline float Object::Angle()
{
  return this->angle_;
}

inline float Object::aspectRatio()
{
  return this->aspect_ratio_;
}

inline float Object::Area()
{
  return this->area_;
}

inline cv::Point2f* Object::Vertex()
{
  return this->vertex_;
}

inline cv::Point2f Object::Vertex(const int& _i)
{
  return this->vertex_[_i];
}

}  // namespace buff

#endif  // !OBJECT_HPP_