#include <RM_Roi.h>

/**
 * @brief Construct a new rm roi::rm roi object
 * @param  _roi_config      roi config information
 */
RM_ROI::RM_ROI(const Roi_Cfg& _roi_config)
{
    this->roi_config = _roi_config;

    this->last_armor_rect = RotatedRect();
    this->tl              = Point2f(0.f, 0.f);
    this->width           = 0;
    this->height          = 0;
}

/**
 * @brief Update param of roi_img
 * @param  _src             src_img
 * @param  is_lost_target   flag of target lost
 * @param  cnt              lost target times
 * @param  last_roi_rect    rectangle of last roi
 */
void RM_ROI::updateROI(Mat&              _src,
                       bool              is_lost_target,
                       short int         cnt,
                       const RotatedRect last_roi_rect)
{
    Point _tl;
    int   W, H;
    // 不丢失目标
    if (!is_lost_target) {
        // 数字识别roi TODO

        if (cnt <= this->roi_config.FIRST_LOSE_CNT_MAX) {
            //  频闪处理 <=2s
            W = last_roi_rect.boundingRect().width * 4;
            H = static_cast<int>(last_roi_rect.boundingRect().height * 4.5);
        }
        else if (this->roi_config.FIRST_LOSE_CNT_MAX < cnt &&
                 cnt <= this->roi_config.SECOND_LOSE_CNT_MAX) {
            // 小陀螺丢失处理 <4s
            W = last_roi_rect.boundingRect().width * 6;
            H = static_cast<int>(last_roi_rect.boundingRect().height * 5.5);
        }

        _tl = Point2f(
            last_roi_rect.center.x - W * 0.5f - this->last_armor_rect.boundingRect().height * 0.5,
            last_roi_rect.center.y - H * 0.5f - this->last_armor_rect.boundingRect().height * 0.5);
        this->safeTreatment(_tl, W, H, last_roi_rect, _src.cols, _src.rows);
        Rect roi = Rect(_tl.x, _tl.y, W, H);
        // 数据更新
        _src(roi).copyTo(roi_img);
        this->inputParam(last_roi_rect, _tl, W, H);
        // 原图上画出roi边界
        rectangle(_src, roi, Scalar(255, 255, 255), 2, 8, 0);
    }
    else {
        // 丢失目标
        // 恢复全图
        _src.copyTo(roi_img);
        // 数据更新
        this->resetParam();
    }
}

/**
 * @brief return top left corner vertex of roi
 * @return Point2f
 */
Point2f RM_ROI::ROItl()
{
    return this->tl;
}

/**
 * @brief Carry out safe treatment
 * @param  _tl              top left corner vertex
 * @param  _W               width
 * @param  _H               height
 * @param  _last_roi_rect   roi rectangle of previous frame
 * @param  _src_W           width of src_img
 * @param  _src_H           height of src_img
 */
void RM_ROI::safeTreatment(Point&             _tl,
                           int&               _W,
                           int&               _H,
                           const RotatedRect& _last_roi_rect,
                           const int&         _src_W,
                           const int&         _src_H)
{
    if (_tl.x < 0) {
        _tl.x = 0;
        //如果左上角点x超出尺寸，x =  0
    }

    if (_tl.y < 0) {
        _tl.y = 0;
        //如果左上角点y超出尺寸，y = 0
    }

    if (_last_roi_rect.center.x - _W * 0.5f < 0.f) {
        _W = _W + static_cast<int>(_last_roi_rect.center.x - _W * 0.5f);
        // _W = static_cast<int>(_last_roi_rect.center.x)+ _W*0.5f;
        // 如果宽度超出了左边界，设置为到边界的值
    }
    if (_tl.x + _W > _src_W) {
        _W = _W - (_tl.x + _W - _src_W);
        // _W = _src_W - _tl.x;
        // 如果宽度超出了右边界，设置为到边界的值
    }
    if (_last_roi_rect.center.y - _H * 0.5f < 0.f) {
        _H = _H + static_cast<int>(_last_roi_rect.center.y - _H * 0.5f);
        // 如果高度超出了上边界，设置为到边界的值
    }
    if (_tl.y + _H > _src_H) {
        _H = _H - (_tl.y + _H - _src_H);
        // 如果高度超出了下边界，设置为到边界的值
    }

    if (_W < 0) {
        _W = 1;
    }

    if (_H < 0) {
        _H = 1;
    }
    std::cout << "W :" << _W << std::endl;
    std::cout << "H :" << _H << std::endl;
}

/**
 * @brief return roi config information
 * @return Roi_Cfg&
 */
Roi_Cfg& RM_ROI::returnRoiConfig()
{
    return this->roi_config;
}

/**
 * @brief update param for roi
 * @param  _last_roi_rect   save roi rectangle as previous frame
 * @param  _tl              top left corner vertex
 * @param  _width           width of roi
 * @param  _height          height of roi
 */
void RM_ROI::inputParam(const RotatedRect& _last_roi_rect,
                        const Point2f&     _tl,
                        const int&         _width,
                        const int&         _height)
{
    // 装甲矩阵
    this->last_armor_rect = _last_roi_rect;
    // 左上角点
    this->tl = _tl;
    // 宽度
    this->width = _width;
    // 高度
    this->height = _height;
}

/**
 * @brief reset param
 */
void RM_ROI::resetParam()
{
    this->last_armor_rect = RotatedRect();
    this->tl              = Point2f(0.f, 0.f);
    this->width           = 0;
    this->height          = 0;
}
