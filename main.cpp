#include <iostream>

#include "camera/rm_video_capture.h"
#include "rm_armor/rm_armor.hpp"
#include "rm_solvepnp/rm_solvepnp.hpp"
#include "roi/rm_roi.h"
#include "serial/rm_serial_port.h"
using namespace std;
using namespace cv;

int main() {
  Rm_Solvepnp pnp(
      "/home/xx/workspace/RM_code-1/RM_2020_vision_code/Basis/RM_VideoCapture/"
      "cameraParams/cameraParams_554.xml",
      "rm_solvepnp/pnp_config.xml");
  cv::Mat src_img;

  cv::VideoCapture cap("/home/xx/下载/视频/效果图/armor_2.avi");
  RM_ArmorDetector armor("rm_armor/armor_config.xml");
  SerialPort serial("/home/xx/桌面/tmp/p4p/serial/serial_config.xml");
  mv_camera::RM_VideoCapture mv_capture_(mv_camera::CameraParam(
      0, mv_camera::RESOLUTION_1280_X_1024, mv_camera::EXPOSURE_800));
  while (true) {
    if (mv_capture_.isindustryimgInput()) {
      src_img = mv_capture_.image();
    }
    if (src_img.empty()) {
      cout << "没有图像" << endl;
      return 0;
    }
    serial.updateReceiveInformation();
    cout << serial.receiveYaw() << endl;
    imshow("dd", src_img);
    if (armor.run_Armor(src_img, serial.returnReceiceColor())) {
    }
    mv_capture_.cameraReleasebuff();
    if (cv::waitKey(1) == 'q') {
      return 0;
    }
    armor.free_Memory();
  }
  return 0;
}
