#include "connector.hpp"

Connector::Connector() {}

Connector::~Connector() {}

void Connector::run() {
  model module_("mnist-8.onnx");
  while (true) {
    if (mv_capture_.isindustryimgInput()) {
      src_img_ = mv_capture_.image();
    } else {
      cap_.read(src_img_);
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
      if (cv::waitKey(0) == 'q') {
        return;
      }
    }
  }
}