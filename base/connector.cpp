#include "connector.hpp"

Connector::Connector() {}

Connector::~Connector() {}

void Connector::run() {
  mv_camera::RM_VideoCapture mv_capture_(mv_camera::CameraParam(
      1, mv_camera::RESOLUTION_1280_X_800, mv_camera::EXPOSURE_600));
  angle_solve::RM_Solvepnp pnp_(
      "devices/camera/cameraParams/cameraParams_407.xml",
      "module/angle_solve/pnp_config.xml");
  armor::RM_ArmorDetector armor_("module/armor/armor_config.xml");
  serial_port::SerialPort serial_("devices/serial/serial_config.xml");
  cv::VideoCapture cap("/home/xx/下载/视频/效果图/armor_1.avi");
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
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          }

          break;
        case serial_port::ENERGY_AGENCY:
          break;

        case serial_port::SENTRY_MODE:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          }

          break;
        case serial_port::BASE_MODE:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          }

          break;
        default:
          if (armor_.run_Armor(src_img_, serial_.returnReceiceColor())) {
            pnp_.run_Solvepnp(serial_.returnReceiveBulletVolacity(),
                              armor_.return_Final_Armor_Distinguish(0),
                              armor_.return_Final_Armor_RotatedRect(0));
          }

          break;
      }
      serial_.updataWriteData(pnp_.returnYawAngle(), pnp_.returnPitchAngle(),
                              pnp_.returnDepth(), armor_.return_Armor_num(), 0);
      mv_capture_.cameraReleasebuff();
      armor_.free_Memory();
      // usleep(1);
      if (cv::waitKey(1) == 'q') {
        return;
      }
    }
  }
}