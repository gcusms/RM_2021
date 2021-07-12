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

  mv_camera::RM_VideoCapture mv_capture_ =//初始相机的结构体
      mv_camera::RM_VideoCapture(mv_camera::CameraParam(
         1, mv_camera::RESOLUTION_1280_X_800, mv_camera::EXPOSURE_600));

  armor::RM_ArmorDetector armor_ =
      armor::RM_ArmorDetector("module/armor/armor_config.xml");
      
  serial_port::SerialPort serial_ =
      serial_port::SerialPort("devices/serial/serial_config.xml");

  buff::RM_Buff buff_ = buff::RM_Buff("module/RM_Buff/Config/buff_config.xml");

  cv::VideoCapture cap_ =
      cv::VideoCapture("/home/sms/VIDEO/camera_MaxBuff15.avi");



 public:
  void run();
  Connector();
  ~Connector();
};

#endif  // !LINKER_H_