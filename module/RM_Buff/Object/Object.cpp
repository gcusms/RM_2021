#include "Object.hpp"

namespace buff {
Object::Object()
{
  this->rect_         = cv::RotatedRect();
  this->width_        = 0.f;
  this->height_       = 0.f;
  this->index_        = 0;
  this->angle_        = 0.f;
  this->aspect_ratio_ = 0.f;
  this->area_         = 0.f;
  this->rect_.points(this->vertex_);
}

Object::~Object() {}

void Object::inputParams(const std::vector<cv::Point>& _contours)
{
  // 矩形
  this->rect_ = cv::fitEllipse(_contours);
  // 长宽
  if (this->rect_.size.width > this->rect_.size.height) {
    this->width_  = this->rect_.size.width;
    this->height_ = this->rect_.size.height;
  }
  else {
    this->width_  = this->rect_.size.height;
    this->height_ = this->rect_.size.width;
  }
  // 长宽比
  if (this->height_ != 0) {
    this->aspect_ratio_ = this->width_ / this->height_;
  }
  else {
    this->aspect_ratio_ = 0.f;
  }
  // 顶点
  this->rect_.points(this->vertex_);
  // 角度
  this->angle_ = this->rect_.angle;
  // 面积
  this->area_ = cv::contourArea(_contours);
}

float centerDistance(const cv::Point& p1, const cv::Point& p2)
{
  float D =
      static_cast<float>(sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y))));
  return D;
}

int getRectIntensity(const cv::Mat& frame, cv::Rect rect)
{
  if (rect.width < 1 || rect.height < 1 || rect.x < 1 || rect.y < 1 ||
      rect.width + rect.x > frame.cols || rect.height + rect.y > frame.rows)
    return 255;
  cv::Mat roi = frame(cv::Range(rect.y, rect.y + rect.height), cv::Range(rect.x, rect.x + rect.width));
  //    imshow("roi ", roi);
  int average_intensity = static_cast<int>(cv::mean(roi).val[0]);
  roi.release();
  return average_intensity;
}
}  // namespace buff