#include "connector.hpp"

Connector::Connector() {}

Connector::~Connector() {}

void Connector::run() {
  mv_camera::RM_VideoCapture mv_capture_(mv_camera::CameraParam(
      0, mv_camera::RESOLUTION_1280_X_1024, mv_camera::EXPOSURE_400));
  angle_solve::RM_Solvepnp pnp_(
      "/home/xx/workspace/RM_code-1/RM_2020_vision_code/Basis/RM_VideoCapture/"
      "cameraParams/cameraParams_554.xml",
      "rm_solvepnp/pnp_config.xml");
  armor::RM_ArmorDetector armor_("rm_armor/armor_config.xml");
  serial_port::SerialPort serial_(
      "/home/xx/桌面/tmp/p4p/serial/serial_config.xml");
  while (true) {
    if (mv_capture_.isindustryimgInput()) {
      src_img_ = mv_capture_.image();
    }
    if (!src_img_.empty()) {
      serial_.updateReceiveInformation();
      switch (serial_.returnReceiveMode()) {
        case serial_port::SUP_SHOOT:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          } else {
            armor_.free_Memory();
          }
          break;
        case serial_port::ENERGY_AGENCY:
          break;

        case serial_port::SENTRY_MODE:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          } else {
            armor_.free_Memory();
          }
          break;
        case serial_port::BASE_MODE:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          } else {
            armor_.free_Memory();
          }
          break;
        default:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          } else {
            armor_.free_Memory();
          }
          break;
      }
      serial_.updataWriteData(pnp_.returnYawAngle(), pnp_.returnPitchAngle(),
                              pnp_.returnDepth(), armor_.returnSuccessArmor(),
                              0);
      mv_capture_.cameraReleasebuff();
      if (cv::waitKey(1) == 'q') {
        return;
      }
    }
  }
}