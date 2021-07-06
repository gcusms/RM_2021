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
    first_temp = count;

  } else if (count == 2) {
  } else if (count == 3) {
    if (top_data_.first_tan_angle > 0) {
      if (_armor_data.tan_angle < 0) {
        status = RIGHT;
      } else {
        if (top_data_.first_tan_angle - _armor_data.tan_angle > 0) {
          status = LEFT;
        } else {
          status = RIGHT;
        }
      }
    } else {
      if (_armor_data.tan_angle > 0) {
        status = LEFT;
      } else {
        if (top_data_.first_tan_angle - _armor_data.tan_angle > 0) {
          status = RIGHT;
        } else {
          status = LEFT;
        }
      }
    }
  } else if (count > 3 && count < 500) {
    if (_armor_data.tan_angle - top_data_.first_tan_angle < 0.2) {
      if (count - first_temp > min_cycle_count /* &&
          count - first_temp < max_cycle_count */) {
        top_data_.shooting_cycle = count - first_temp;
        top_data_.top_cycle = 4 * top_data_.shooting_cycle;
        top_data_.status = status;
      } else {
        countInitializ();
        status = STOP;
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