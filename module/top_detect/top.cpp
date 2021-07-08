#include "module/top_detect/top.hpp"

void Armor_Top::countInitializ() { count = 0; }

void Armor_Top::topStatusInitializ() {
  top_data_.status = STOP;
  top_data_.is_shooting = 0;
  top_data_.top_cycle = 0;
  top_data_.first_tan_angle = 0.f;
}

Top_Status Armor_Top::run_Top(cv::Mat &_src_img,
                              armor::Armor_Data _armor_data) {
  ++count;  //计数
  if (count == 1) {
    top_data_.first_tan_angle = _armor_data.tan_angle;
  } else if (count > 1 && count < max_cycle_count) {
    if (_armor_data.tan_angle < top_data_.first_tan_angle + 0.5 &&
        _armor_data.tan_angle > top_data_.first_tan_angle - 0.5) {
      if (count > min_cycle_count) {
        if (top_data_.status == TRANSITION || top_data_.status == ISTOP) {
          top_data_.status = ISTOP;
          top_data_.shooting_cycle = (count + transition_cycle) / 2;
          top_data_.top_cycle = 4 * top_data_.shooting_cycle;
          countInitializ();
        } else {
          top_data_.status = TRANSITION;
          transition_cycle = count;
          countInitializ();
        }
      } else {
        top_data_.status = STOP;
        countInitializ();
      }
    }
    std::cout << "周期 = " << top_data_.top_cycle << std::endl;
  } else {
    countInitializ();
  }
  return top_data_.status;
}

Armor_Top::Armor_Top() {}
Armor_Top::~Armor_Top() {}