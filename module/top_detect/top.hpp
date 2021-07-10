#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "module/armor/rm_armor.hpp"

enum Top_Status {
  /**
   * @brief STOP       停止
   * @brief TRANSITION 过渡时期
   * @brief ISTOP      小陀螺
   */
  STOP,
  TRANSITION,
  ISTOP,
};

struct Top_Data {
  Top_Status status = STOP;   // 陀螺状态
  int is_shooting = 0;        // 开火键
  int shooting_cycle = 0;     // 射击周期
  int top_cycle = 0;          // 陀螺周期
  float first_tan_angle = 0;  // 第一次检测tan角度
};
class Armor_Top {
 private:
  Top_Data top_data_;         // 小陀螺状态
  int count = 0;              // 运行次数计数
  int min_cycle_count = 5;    // 最小周期
  int max_cycle_count = 200;  // 最大周期
  int first_temp;             // 第一次检测到位置
  int transition_cycle = 0;   // 过渡周期
  Top_Status status = STOP;

 public:
  /**
   * @brief 主函数
   *
   * @param _src_img 原图像
   * @param
   * @return Top_Data
   */
  Top_Status run_Top(cv::Mat &_src_img, float _data);
  /**
   * @brief 小陀螺状态清零
   *
   */
  void topStatusInitializ();
  /**
   * @brief 计数器清零
   *
   */
  void countInitializ();
  Armor_Top();
  ~Armor_Top();
};