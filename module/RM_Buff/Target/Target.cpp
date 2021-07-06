#include "Target.hpp"

namespace buff
{
  
Target::Target()
{
  fan_blade_ = FanBlade();
  fan_armor_ = FanArmor();
  this->angle_ =0.f;
  this->diff_angle_ = 0.f;
  this->type_       = UNKNOW;
  this->area_ratio_ = 0.f;
  this->points_2d_.reserve(4);
  this->delta_height_point_ = cv::Point2f(0.f, 0.f);
}

Target::~Target() {}

void Target::inputParams(const FanBlade& _fan_blade, const FanArmor& _fan_armor)
{
  // 内轮廓
  this->fan_armor_ = _fan_armor;

  // 外轮廓
  this->fan_blade_ = _fan_blade;

  // 内外轮廓角度差
  this->diff_angle_ = fabsf(this->fan_armor_.Angle() - this->fan_blade_.Angle());

  // 扇叶角度= 扇叶和装甲板的连线 TODO:未使用
  this->angle_ = atan2((this->fan_armor_.Rect().center.y - this->fan_blade_.Rect().center.y),
                       (this->fan_armor_.Rect().center.x - this->fan_blade_.Rect().center.x)) *
                 180 / static_cast<float>(CV_PI);
  // 过零处理
  if (this->angle_ < 0.f) {
    this->angle_ += 360.f;
  }

  // 设置默认运行模式：未知
  this->type_ = UNKNOW;

  // 面积比例(方案一)
  this->area_ratio_ = this->fan_armor_.Rect().size.area() / this->fan_blade_.Rect().size.area();

  // 面积比例(方案二)
  // this->area_ratio_ = this->fan_armor_.Area() / this->fan_blade_.Area();
}

void Target::updateVertex(cv::Mat& _img)
{
  this->points_2d_.clear();

  cv::Point2f point_up_center   = (this->fan_armor_.Vertex(0) + this->fan_armor_.Vertex(1)) * 0.5;
  cv::Point2f point_down_center = (this->fan_armor_.Vertex(2) + this->fan_armor_.Vertex(3)) * 0.5;

  float up_distance   = centerDistance(point_up_center, this->fan_blade_.Rect().center);
  float down_distance = centerDistance(point_down_center, this->fan_blade_.Rect().center);

  if (up_distance > down_distance) {
    // 0 1
    // 3 2
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(0));
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(1));
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(2));
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(3));
  }
  else {
    // 2 3
    // 1 0
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(2));
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(3));
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(0));
    this->points_2d_.emplace_back(this->fan_armor_.Vertex(1));
  }

  // 画出各个顶点 红黄蓝绿
  cv::circle(_img, this->points_2d_[0], 5, cv::Scalar(0, 0, 255), -1);
  cv::circle(_img, this->points_2d_[1], 5, cv::Scalar(0, 255, 255), -1);
  cv::circle(_img, this->points_2d_[2], 5, cv::Scalar(255, 155, 0), -1);
  cv::circle(_img, this->points_2d_[3], 5, cv::Scalar(0, 255, 0), -1);
}

void Target::setType(const ObjectType& _type)
{
  this->type_ = _type;
}

void Target::setType(cv::Mat& _bin_img, cv::Mat& _img)
{
  // 上层中心点 和 下层中心点
  cv::Point2f point_up_center   = (this->fan_armor_.Vertex(0) + this->fan_armor_.Vertex(1)) * 0.5;
  cv::Point2f point_down_center = (this->fan_armor_.Vertex(2) + this->fan_armor_.Vertex(3)) * 0.5;

  // 上层离外轮廓中点距离
  // 下册离外轮廓中点距离
  // TODO：待修改为优化方案，不用重复判断方向
  float up_distance   = centerDistance(point_up_center, this->fan_blade_.Rect().center);
  float down_distance = centerDistance(point_down_center, this->fan_blade_.Rect().center);

  // 计算装甲板的高度差，且方向朝向圆心
  cv::Point2f vector_height;
  if (up_distance > down_distance) {
    vector_height = this->fan_armor_.Vertex(0) - this->fan_armor_.Vertex(3);
  }
  else {
    vector_height = this->fan_armor_.Vertex(3) - this->fan_armor_.Vertex(0);
  }

  // 这里记得分清楚应该是在哪个点往哪个方向移动多少距离进行框取小ROI
  // 计算两个小roi的中心点
  cv::Point left_center  = this->vector2DPoint(3) - vector_height;
  cv::Point right_center = this->vector2DPoint(2) - vector_height;

  // 创建roi并绘制
  int width  = 10;
  int height = 5;

  cv::Point left1 = cv::Point(left_center.x - width, left_center.y - height);
  cv::Point left2 = cv::Point(left_center.x + width, left_center.y + height);

  cv::Point right1 = cv::Point(right_center.x - width, right_center.y - height);
  cv::Point right2 = cv::Point(right_center.x + width, right_center.y + height);

  cv::Rect left_rect(left1, left2);     // 画出左边小roi
  cv::Rect right_rect(right1, right2);  // 画出右边小roi

  // 计算光线强度
  int left_intensity  = getRectIntensity(_bin_img, left_rect);
  int right_intensity = getRectIntensity(_bin_img, right_rect);

  if (left_intensity <= 15 && right_intensity <= 15) {
    this->type_ = INACTION;
  }
  else {
    this->type_ = ACTION;
  }
}

void Target::displayInactionTarget(cv::Mat& _img)
{
  if (this->type_ == INACTION) {
    for (int k = 0; k < 4; ++k) {
      cv::line(_img, this->fan_armor_.Vertex(k), this->fan_armor_.Vertex((k + 1) % 4),
           cv::Scalar(0, 255, 0), 2);
      cv::line(_img, this->fan_blade_.Vertex(k), this->fan_blade_.Vertex((k + 1) % 4),
           cv::Scalar(0, 255, 255), 2);
    }
  }
}

}