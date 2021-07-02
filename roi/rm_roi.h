#ifndef IMAGE_ROI_H_
#define IMAGE_ROI_H_

#include "abstract_image_roi.h"

namespace roi {

class ImageRoi : public Abstract_ImageRoi
{
 private:

 public:
  ImageRoi(/* args */) { }
  ~ImageRoi() { Abstract_ImageRoi::~Abstract_ImageRoi();}

  /**
   * @brief   ROI 区域安全处理，防止越界
   * @details 第一种处理方式，如果超界则将同样大小的 ROI 区域固定在图像边缘
   * @param _input_img          输入图像，用来表示边界
   * @param _rect         需要处理的 ROI 区域
   * @return cv::Rect 
   */
  cv::Rect makeRectSafe_Fixed(const cv::Mat         &_input_img,
                              const cv::RotatedRect &_r_rect);

  /**
   * @brief ROI 区域安全处理，防止越界
   * @details 第二种处理方式，如果超界，则将 ROI 区域裁剪大小后返回
   * @param _input_img          输入图像，用来表示边界
   * @param _rect         需要处理的 ROI 区域
   * @return cv::Rect 
   */
  cv::Rect makeRectSafe_Tailor( const cv::Mat         &_input_img,
                                const cv::RotatedRect &_r_rect);
};

} // namespace roi

#endif // !IMAGE_ROI_H_
