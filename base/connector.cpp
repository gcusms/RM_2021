#include "connector.hpp"

Connector::Connector() {}

Connector::~Connector() {}

void Connector::run() {
  mv_camera::RM_VideoCapture mv_capture_(mv_camera::CameraParam(
      0, mv_camera::RESOLUTION_1280_X_800, mv_camera::EXPOSURE_600));
  armor::RM_ArmorDetector armor_("module/armor/armor_config.xml");
  serial_port::SerialPort serial_("devices/serial/serial_config.xml");
  cv::VideoCapture cap("/home/xx/下载/视频/效果图/armor_1.avi");
  buff::RM_Buff buff ("module/RM_Buff/Config/buff_config.xml");

  while (true) {
    if (mv_capture_.isindustryimgInput()) {
      src_img_ = mv_capture_.image();
    } else {
      cap.read(src_img_);
    }
    if (!src_img_.empty()) {
      serial_.updateReceiveInformation();
      switch (serial_.returnReceiveMode()) {
        case serial_port::SUP_SHOOT:
          serial_.rmSerialWrite(
              armor_.run_Armor(src_img_, serial_.returnReceive()));
          break;
        case serial_port::ENERGY_AGENCY:
          break;

        case serial_port::SENTRY_MODE:
          serial_.rmSerialWrite(
              armor_.run_Armor(src_img_, serial_.returnReceive()));

          break;
        case serial_port::BASE_MODE:
          serial_.rmSerialWrite(
              armor_.run_Armor(src_img_, serial_.returnReceive()));

          break;
        default:
          serial_.rmSerialWrite(
              armor_.run_Armor(src_img_, serial_.returnReceive()));

          break;
      }
      mv_capture_.cameraReleasebuff();
      armor_.free_Memory();
      // usleep(1);
      if (cv::waitKey(1) == 'q') {
        return;
      }
    }
  }
}