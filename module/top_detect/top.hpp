#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

namespace top {
enum Top_Status {
  /**
   * @brief STOP       不是
   * @brief ISTOP      小陀螺
   */
  STOP,
  ISTOP,
};

struct Top_Data {
  Top_Status status = STOP;  // 陀螺状态
  int is_shooting = 0;       // 开火键
  int shooting_cycle = 0;    // 射击周期
  int top_cycle = 0;         // 陀螺周期
};
class Armor_Top {
 private:
  Top_Data top_data_;          // 小陀螺状态
  int count = 0;               // 运行次数计数
  int min_cycle_count_ = 5;    // 最小周期
  int max_cycle_count_ = 200;  // 最大周期
  float lost_data_;            // 上个数据
  float data_different_;       // 数据总差值
  int judge_count_ = 0;        // 判断计数
  Top_Status status_ = STOP;   // 陀螺状态
  int minimum_gap_ = 50;       // 最小差值
  int minimum_error_ = 50;     // 最小误差
  int true_floor_ = 2;         // 通过下限
  int true_upper_ = 5;         // 通过上限
  cv::Mat top_trackbar_ = cv::Mat::zeros(1, 300, CV_8UC1);

 public:
  /**
   * @brief 小陀螺判断
   *
   * @param _data 陀螺仪yaw数据
   * @return Top_Status
   */
  Top_Status run_Top(float _data);
  /**
   * @brief 小陀螺状态清零
   *
   */
  void topStatusInitializ();
  /**
   * @brief 计数器清零
   *
   */
  inline void countInitializ() { count = 0; }
  Armor_Top();
  ~Armor_Top();
};
}  // namespace top