#ifndef LINKER_H_
#define LINKER_H_

#include "devices/camera/rm_video_capture.hpp"
#include "devices/serial/rm_serial_port.hpp"
#include "module/RM_Buff/RM_Buff.hpp"
#include "module/angle_solve/rm_solve_pnp.hpp"
#include "module/armor/rm_armor.hpp"
class Connector {
 private:
  cv::Mat src_img_;
  mv_camera::RM_VideoCapture mv_capture_ =
      mv_camera::RM_VideoCapture(mv_camera::CameraParam(
          0, mv_camera::RESOLUTION_1280_X_800, mv_camera::EXPOSURE_600));
  armor::RM_ArmorDetector armor_ =
      armor::RM_ArmorDetector("module/armor/armor_config.xml");
  serial_port::SerialPort serial_ =
      serial_port::SerialPort("devices/serial/serial_config.xml");
  buff::RM_Buff buff = buff::RM_Buff("module/RM_Buff/Config/buff_config.xml");

 public:
  void run();
  Connector();
  ~Connector();
};

#endif  // !LINKER_H_