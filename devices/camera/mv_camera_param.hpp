#ifndef CAMERA_PARAM_H_
#define CAMERA_PARAM_H_

namespace mv_camera {

enum EXPOSURETIME {
  /**
   * @brief 预设值曝光时间
   *
   */
  EXPOSURE_5000 = 5000,
  EXPOSURE_2500 = 2500,
  EXPOSURE_1200 = 1200,
  EXPOSURE_800 = 800,
  EXPOSURE_600 = 600,
  EXPOSURE_400 = 400,
};

enum RESOLUTION {
  /**
   * @brief 预设置分辨率
   * @details 由于相机的硬件属性，预设分辨率需要满足
   *          宽为 16 的倍数，高为 2 的倍数
   */
  RESOLUTION_1280_X_1024,  // 默认 1280 x 1024
  RESOLUTION_1280_X_800,   // 1280 x 800
  RESOLUTION_640_X_480,    // 640 x 480
};

struct Camera_Resolution {
  int cols;
  int rows;

  /**
   * @param[in] _cols   相机分辨率 宽
   * @param[in] _rows   相机分辨率 高
   */
  Camera_Resolution(const mv_camera::RESOLUTION _resolution) {
    switch (_resolution) {
      case mv_camera::RESOLUTION::RESOLUTION_1280_X_1024: {
        cols = 1280;
        rows = 1024;
        break;
      }
      case mv_camera::RESOLUTION::RESOLUTION_1280_X_800: {
        cols = 1280;
        rows = 800;
        break;
      }
      case mv_camera::RESOLUTION::RESOLUTION_640_X_480: {
        cols = 640;
        rows = 480;
        break;
      }
      default: {
        cols = 1280;
        rows = 800;
        break;
      }
    }
  }
};
struct CameraParam {
  int camera_mode;
  int camera_exposuretime;
  Camera_Resolution resolution;

  /**
   * @brief Construct a new User Param object
   *
   * @param[in] _camera_mode              选择相机模式
   * @param[in] _resolution               选择相机分辨率
   * @param[in] _camera_exposuretime      设置相机曝光时间
   */
  CameraParam(const int _camera_mode, const mv_camera::RESOLUTION _resolution,
              const mv_camera::EXPOSURETIME _camera_exposuretime)
      : camera_mode(_camera_mode),
        resolution(_resolution),
        camera_exposuretime(_camera_exposuretime) {}
};

}  // namespace mv_camera

#endif  // !CAMERA_PARAM_H_
