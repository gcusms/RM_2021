#ifndef LINKER_H_
#define LINKER_H_

#include "devices/camera/rm_video_capture.hpp"
#include "devices/serial/rm_serial_port.hpp"
#include "module/angle_solve/rm_solve_pnp.hpp"
#include "module/armor/rm_armor.hpp"
#include "module/top_detect/top.hpp"
class Connector {
 private:
  cv::Mat src_img_;

 public:
  void run();
  Connector();
  ~Connector();
};

#endif  // !LINKER_H_