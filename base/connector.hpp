#ifndef LINKER_H_
#define LINKER_H_

#include "camera/rm_video_capture.hpp"
#include "rm_armor/rm_armor.hpp"
#include "rm_solvepnp/rm_solvepnp.hpp"
#include "serial/rm_serial_port.hpp"

class Connector {
 private:
  Mat src_img_;

 public:
  void run();
  Connector();
  ~Connector();
};

#endif  // !LINKER_H_