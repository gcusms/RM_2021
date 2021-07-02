#include "connector.hpp"

Connector::Connector() {}

Connector::~Connector() {}

void Connector::run() {
  mv_camera::RM_VideoCapture mv_capture_(mv_camera::CameraParam(
      0, mv_camera::RESOLUTION_1280_X_1024, mv_camera::EXPOSURE_400));
  Rm_Solvepnp pnp(
      "/home/xx/workspace/RM_code-1/RM_2020_vision_code/Basis/RM_VideoCapture/"
      "cameraParams/cameraParams_554.xml",
      "rm_solvepnp/pnp_config.xml");
  RM_ArmorDetector armor("rm_armor/armor_config.xml");
  SerialPort serial("/home/xx/桌面/tmp/p4p/serial/serial_config.xml");
  while (true) {
    if (mv_capture_.isindustryimgInput()) {
      src_img_ = mv_capture_.image();
    }
    if (!src_img_.empty()) {
      serial.updateReceiveInformation();
      switch (serial.returnReceiveMode()) {
        case SUP_SHOOT:
          if (armor.run_Armor(src_img_, serial.returnReceiceColor())) {
            pnp.run_Solvepnp(serial.returnReceiveBulletVolacity(),
                             armor.return_Final_Armor_Distinguish(0),
                             armor.return_Final_Armor_RotatedRect(0));
          } else {
            armor.free_Memory();
          }
          break;
        case ENERGY_AGENCY:
          break;

        case SENTRY_MODE:
          if (armor.run_Armor(src_img_, serial.returnReceiceColor())) {
            pnp.run_Solvepnp(serial.returnReceiveBulletVolacity(),
                             armor.return_Final_Armor_Distinguish(0),
                             armor.return_Final_Armor_RotatedRect(0));
          } else {
            armor.free_Memory();
          }
          break;
        case BASE_MODE:
          if (armor.run_Armor(src_img_, serial.returnReceiceColor())) {
            pnp.run_Solvepnp(serial.returnReceiveBulletVolacity(),
                             armor.return_Final_Armor_Distinguish(0),
                             armor.return_Final_Armor_RotatedRect(0));
          } else {
            armor.free_Memory();
          }
          break;
        default:
          if (armor.run_Armor(src_img_, serial.returnReceiceColor())) {
            pnp.run_Solvepnp(serial.returnReceiveBulletVolacity(),
                             armor.return_Final_Armor_Distinguish(0),
                             armor.return_Final_Armor_RotatedRect(0));
          } else {
            armor.free_Memory();
          }
          break;
      }
      serial.updataWriteData(pnp.returnYawAngle(), pnp.returnPitchAngle(),
                             pnp.returnDepth(), armor.returnSuccessArmor(), 0);
      mv_capture_.cameraReleasebuff();
      if (cv::waitKey(1) == 'q') {
        return;
      }
    }
  }
}