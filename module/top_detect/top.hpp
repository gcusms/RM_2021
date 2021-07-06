#ifndef SHOOTTUOLUO_H
#define SHOOTTUOLUO_H

#include <opencv2/opencv.hpp>

#include "module/angle_solve/rm_solve_pnp.hpp"
#include "module/armor/rm_armor.hpp"
#include "module/filter/filter.hpp"

//击打缓冲计算返回
typedef struct {
  float pitch;
  float yaw;
  float t;  //击打弹道时间
  float angle;
} Angle_t;

//陀螺状态
enum TopStatus {
  STILL,   //静止
  SWITCH,  //切换
  RUN      //同块运动
};

//陀螺转动方向
enum TopRunDirection { LEFT, RIGHT };
//陀螺数据
struct TopData {
  bool isTuoluo = false;  //是否为陀螺
  float R;                //半径
  float angle;            //当前角度
  cv::Point2f center;
  TopStatus status;              //陀螺当前的状态
  TopRunDirection runDirection;  //转动方向
  float spinSpeed;               //角速度
};

class ShootTuoluo {
 private:
  double tz_armor_width_;
  double tz_normal;

 public:
  TopData getTuoluoData(cv::Mat Src, armor::Armor_Data BestArmor);
  ShootTuoluo();

 private:
  double getAngle(armor::Armor_Data BestArmor);
  void firstSetTuoluo(double angle, armor::Armor_Data armor);
  //第二种陀螺检测方案
  bool ContinueSetTuoluo(cv::Mat Src, double& angle,
                         armor::Armor_Data BestArmor, double& R,
                         cv::Point2f& center, TopStatus& status,
                         TopRunDirection& direction, float& spinSpeed);
  void drawImage(cv::Mat Src, double angle, cv::Point2f object, double R,
                 float armorWidth);
};

struct Angle_tz {
  double angle = 90;
  float tz = 0;
  float r = 0;
};

#endif  // SHOOTTUOLUO_H
