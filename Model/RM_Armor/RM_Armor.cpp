#include "Model/RM_Armor/RM_Armor.h"

/**
 * @brief Construct a new rm armor::rm armor object
 * @param  _armor_config    初始化自瞄参数配置表
 * @param  _roi_config      初始化 ROI 参数配置表
 */
RM_Armor::RM_Armor(const Armor_Cfg& _armor_config, const Roi_Cfg& _roi_config)
    : armor_config(_armor_config), armor_roi(_roi_config), armor(armor_config.candidate_armor_cfg)
{
    // 预制目标装甲板的四个顶点存储空间大小
    this->target_2d.reserve(4);
}

RM_Armor::~RM_Armor() {}

/**
 * @brief 存入参数
 * @param  _rect            灯条旋转矩形
 * @param  _index           灯条索引号
 * @param  _contours        灯条的点集
 * @param  _roi             截取的 ROI
 */
void Light::inputparam(const RotatedRect&   _rect,
                       const size_t&        _index,
                       const vector<Point>& _contours,
                       RM_ROI&              _roi,
                       const Point&         _img_center)
{
    this->width  = MIN(_rect.size.width, _rect.size.height);
    this->height = MAX(_rect.size.width, _rect.size.height);

    this->rect = RotatedRect(_rect.center + _roi.ROItl(), _rect.size, _rect.angle);

    this->aspect_ratio = _rect.size.width / _rect.size.height;

    if (_rect.angle >= 0.f && _rect.angle <= 90.f) {
        this->angle = _rect.angle + 90.f;
    }
    else if (_rect.angle > 90.f && _rect.angle <= 180.f) {
        this->angle = _rect.angle - 90.f;
    }

    this->perimeter = arcLength(_contours, true);

    this->index = _index;

    this->rect.points(vertex);

    this->dist_to_center = centerDistance(_rect.center, _img_center);
}

/**
 * @brief  更新索引号
 * @param  _index           索引号
 */
void Light::updateIndex(const size_t& _index)
{
    this->index = _index;
}

/**
 * @brief Construct a new Candidate Armor:: Candidate Armor object
 * @param  _candidate_armor_configMy 初始化候选装甲板参数配置表
 */
CandidateArmor::CandidateArmor(const Candidate_Armor_Cfg& _candidate_armor_config)
    : candidate_armor_config(_candidate_armor_config)
{
    this->armor_type_th  = this->candidate_armor_config.ARMOR_TYPE_TH;
    this->type           = SMALL_ARMOR;
    this->rect           = cv::RotatedRect();
    this->aspect_ratio   = 0.f;
    this->dist_to_center = 0;
    this->angle          = 0.f;
    this->diff_angle     = 0.f;
    this->index          = 0;
    this->width          = 0.f;
    this->height         = 0.f;
}

/**
 * @brief 存入拟合装甲板参数
 * @param  _left_light      左灯条
 * @param  _right_light     右灯条
 * @param  _index           装甲板索引
 * @param  _roi             ROI
 * @param  _img_center      图像中心点（默认）
 */
void CandidateArmor::inputparam(const Light&  _left_light,
                                const Light&  _right_light,
                                const size_t& _index,
                                RM_ROI&       _roi,
                                const Point&  _img_center)
{
    // 拟合初步装甲板，获取信息
    this->rect = this->draftTentativeArmor(_left_light, _right_light);

    // 装甲板宽度 可能需要修正

    // 装甲板中点距离图像中点信息
    this->dist_to_center = centerDistance(this->rect.center, _img_center);

    // 得到装甲板的旋转矩阵
    this->rect = RotatedRect(this->rect.center, rect.size, rect.angle);

    // 更新数据
    this->updateCandidateArmor(_left_light, _right_light, _index);
    // 计算深度：略
}

/**
 * @brief 拟合初步装甲板
 * @param  _left_light      左灯条
 * @param  _right_light     右灯条
 * @return RotatedRect      拟合装甲板
 */
RotatedRect CandidateArmor::draftTentativeArmor(const Light& _left_light, const Light& _right_light)
{
    this->center_tmp = (_left_light.returnRect().center + _right_light.returnRect().center) * 0.5;

    this->distance_tmp =
        centerDistance(_left_light.returnRect().center, _right_light.returnRect().center);

    this->w_tmp =
        distance_tmp + (_left_light.returnWidth() * 0.5 + _right_light.returnWidth() * 0.5);

    this->h_tmp = MAX(_left_light.returnHeight(), _right_light.returnHeight());

    this->angle_tmp =
        atan2((_left_light.returnRect().center.y - _right_light.returnRect().center.y),
              (_left_light.returnRect().center.x - _right_light.returnRect().center.x));

    this->rect_tmp =
        RotatedRect(center_tmp, Size2f(w_tmp, h_tmp), angle_tmp * 180 / static_cast<float>(CV_PI));

    return this->rect_tmp;
}

/**
 * @brief 更新候选装甲板信息
 * @param  _left_light      左灯条
 * @param  _right_light     右灯条
 * @param  _index           装甲板索引
 */
void CandidateArmor::updateCandidateArmor(const Light&  _left_light,
                                          const Light&  _right_light,
                                          const size_t& _index)
{
    // 更新左右灯条
    this->left_light  = _left_light;
    this->right_light = _right_light;

    // 更新宽度、高度
    this->width  = this->rect.size.width;
    this->height = this->rect.size.height;

    // 更新角度差
    this->diff_angle = fabs(_left_light.returnAngle() - _right_light.returnAngle());

    // 更新角度
    if (this->rect.angle < 0.f) {
        this->angle = 360.f + this->rect.angle;
    }
    else {
        this->angle = this->rect.angle;
    }

    // 计算宽高比
    this->aspect_ratio = this->width / this->height;

    // 判断大小装甲板 需要测试
    this->type = this->judgeArmorSize();

    // 更新当前装甲板序号
    this->index = _index;

    this->rect.points(this->vertex);
}

/**
 * @brief 判断大小装甲板
 * @return armor_size
 * @note SMALL_ARMOR    小
 * @note BIG_ARMOR      大
 */
armor_size CandidateArmor::judgeArmorSize()
{
    if (this->aspect_ratio < this->candidate_armor_config.ARMOR_TYPE_TH) {
        return SMALL_ARMOR;
    }
    else {
        return BIG_ARMOR;
    }
}

/**
 * @brief 总执行函数
 * @param  _frame           处理图像
 * @param  _receive_info    信息传输
 * @return true             有目标
 * @return false            无目标
 */
bool RM_Armor::identificationTask(Mat _frame, RM_Messenger* _messenger)
{
    // 预处理
    this->imageProcessing(_frame, _messenger->getReceiveInfo().my_color);

#ifndef RELEASE
    // Create trackbar
    this->createLightTrackbar();

#    ifdef DEBUG
    // Create trackbar
    this->createOpeimumArmorTrackbar();
#    endif  // DEBUG

#endif  // !RELEASE

#ifdef DEBUG
    Point put_lose_cnt = Point(640, 400);
    putText(this->src_img, to_string(this->lose_target_cnt), put_lose_cnt, FONT_HERSHEY_PLAIN, 30,
            Scalar(0, 255, 0), 10, 8, false);
#endif  // DEBUG

    // 筛选灯条，少于等于一个退出 TODO:更新串口发送信息 坐标，标志位
    if (!this->lightScreenting()) {
#ifdef DEBUG

        // 根据丢包次数来分发送内容，小于5次还是用回之前剩下的，大于5次才清空,同roi的first阈值 TODO

        cout << "X X X 筛选灯条失败 X X X" << endl;
        if (this->lose_target_cnt <= this->armor_config.FIRST_LOSE_CNT_MAX) {
            // 频闪退出
            cout << " 频闪退出 " << endl;
            // 更新标志位为未检测到装甲板
            this->is_last_data_catch = false;
            // 发送上一帧的装甲板
            this->final_armor = this->last_armor;
            this->sortArmorVertex();
            return true;
        }
        else if (this->lose_target_cnt > this->armor_config.FIRST_LOSE_CNT_MAX &&
                 this->lose_target_cnt <= this->armor_config.SECOND_LOSE_CNT_MAX) {
            // 切换目标退出
            cout << " 切换目标退出 " << endl;
            // 更新标志位为未检测到装甲板
            this->is_last_data_catch = false;
            // 发送0
            _messenger->setSendLostInfo();
            return false;
        }
        else {
            // 切换目标退出
            cout << " 丢失目标退出 " << endl;
            // 更新标志位为未检测到装甲板
            this->is_last_data_catch = false;
            // 发送0
            _messenger->setSendLostInfo();
            return false;
        }
#else
        if (lose_target_cnt < 2) {
            this->final_armor = this->last_armor;
        }
        else {
            _messenger->setSendLostInfo();
        }
        cout << "X X X 筛选灯条失败 X X X" << endl;
        return false;

#endif  // DEBUG
    }

    cout << "筛选灯条成功 ! ! !" << endl;

#ifndef RELEASE
    if (this->armor_config.IS_DRAW_LIGHT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("light_img", light_img);
        cout << "🧐 灯条 框选图像已打开 🧐" << endl;
    }
#endif  // !RELEASE

#ifndef RELEASE
    // Create trackbar
    this->createCandidateArmorTrackbar();
#endif  // !RELEASE

    // 筛选候选装甲板，少于一个退出 TODO:更新串口发送信息 坐标，标志位
    if (!this->armorScreenting()) {
#ifdef DEBUG

        // 根据丢包次数来分发送内容，小于5次还是用回之前剩下的，大于5次才清空,同roi的first阈值 TODO

        cout << "X X X 筛选装甲板失败 X X X" << endl;
        if (this->lose_target_cnt <= this->armor_config.FIRST_LOSE_CNT_MAX) {
            // 频闪退出
            cout << " 频闪退出 " << endl;
            // 更新标志位为未检测到装甲板
            this->is_last_data_catch = false;
            // 发送上一帧的装甲板
            this->final_armor = this->last_armor;
            this->sortArmorVertex();

            return true;
        }
        else if (this->lose_target_cnt > this->armor_config.FIRST_LOSE_CNT_MAX &&
                 this->lose_target_cnt <= this->armor_config.SECOND_LOSE_CNT_MAX) {
            // 切换目标退出
            cout << " 切换目标退出 " << endl;
            // 更新标志位为未检测到装甲板
            this->is_last_data_catch = false;
            // 发送0
            _messenger->setSendLostInfo();

            return false;
        }
        else {
            // 切换目标退出
            cout << " 丢失目标退出 " << endl;
            // 更新标志位为未检测到装甲板
            this->is_last_data_catch = false;
            // 发送0
            _messenger->setSendLostInfo();
            return false;
        }
#else
        if (lose_target_cnt < 2) {
            this->final_armor = this->last_armor;
        }
        else {
            _messenger->setSendLostInfo();
        }
        cout << "X X X 筛选装甲板失败 X X X" << endl;
        return false;

#endif  // DEBUG
    }

    cout << "筛选装甲板成功 ! ! !" << endl;

    // 选取最优装甲板,只有一个时可忽略 TODO:更新串口发送信息 坐标，标志位
    this->armorSelectOptimum();

    // 修改有目标标志位
    _messenger->getSendInfo().is_found_target = 1;

    cout << "完成识别任务！！！" << endl;

#ifndef RELEASE
    // 显示装甲板图像
    if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("armor", this->armor_img);
        cout << "🧐 装甲板 框选图像已打开 🧐" << endl;
    }
#endif  // !RELEASE

    return true;
}

/**
 * @brief  预处理
 * @param  _frame           处理图像
 * @param  _my_color        颜色信息
 */
void RM_Armor::imageProcessing(Mat _frame, const int& _my_color)
{
    // 更新图像
    this->src_img = _frame;

#ifndef RELEASE
#    ifdef DEBUG

    if (!is_Lost_target && is_last_data_catch) {
        // 正常尺寸
        if (this->change_buff_cnt > 0) {
            --this->change_buff_cnt;
            this->armor_range = 755;
        }
        else {
            this->armor_range = 100 + this->last_armor.retunrWidth() *
                                          this->armor_config.FIRST_LEVEL_LOSS_RANGE * 0.01;
        }
    }
    else if (!is_Lost_target && !is_last_data_catch &&
             lose_target_cnt <= this->armor_config.FIRST_LOSE_CNT_MAX) {
        // 一级尺寸
        this->armor_range =
            150 + this->last_armor.retunrWidth() * this->armor_config.FIRST_LEVEL_LOSS_RANGE * 0.01;

        this->lose_target_cnt += 1;
    }
    else if (!is_Lost_target && !is_last_data_catch &&
             lose_target_cnt <= this->armor_config.SECOND_LOSE_CNT_MAX) {
        // 二级尺寸
        this->armor_range = 300 + this->last_armor.retunrWidth() *
                                      this->armor_config.SECOND_LEVEL_LOSS_RANGE * 0.01;
        this->lose_target_cnt += 1;
    }
    else {
        // 丢失目标无尺寸限制
        this->armor_range     = 755;
        this->change_buff_cnt = 10;

        this->lose_target_cnt = 0;
        is_Lost_target        = true;
    }

    // else：原图
    this->roi_img = src_img;
#    else
    if (this->armor_config.IS_ENABLE_ROI_RECT == 1)
        if (!is_Lost_target && is_last_data_catch) {
            this->armor_roi.updateROI(this->src_img, this->is_Lost_target, this->lose_target_cnt,
                                      this->last_armor.returnRect());
            roi_img = armor_roi.roi_img;
        }
        else if (!is_Lost_target && !is_last_data_catch &&
                 lose_target_cnt <= this->armor_roi.returnRoiConfig().SECOND_LOSE_CNT_MAX) {
            this->armor_roi.updateROI(this->src_img, this->is_Lost_target, this->lose_target_cnt,
                                      this->last_armor.returnRect());

            roi_img = armor_roi.roi_img;
            this->lose_target_cnt += 1;
        }
        else {
            this->lose_target_cnt = 0;
            is_Lost_target        = true;
            this->armor_roi.updateROI(this->src_img, this->is_Lost_target, this->lose_target_cnt,
                                      this->last_armor.returnRect());
            roi_img = armor_roi.roi_img;
        }
    else {
        // else：原图
        this->armor_roi.resetParam();
        this->roi_img = src_img;
    }
#    endif  // DEBUG
#else
    this->roi_img = src_img;

#endif  // !RELEASE

    this->src_img.copyTo(this->dst_img);

#ifndef RELEASE
    // 调参时需要显示的灯条图像
    if (this->armor_config.IS_DRAW_LIGHT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        this->src_img.copyTo(this->light_img);
    }
    // 调参时需要显示的装甲板图像
    if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        this->src_img.copyTo(this->armor_img);
    }
#endif  // !RELEASE

    // 获取灰度图
    cvtColor(this->roi_img, this->gray_img, COLOR_BGR2GRAY);

    // 选择预处理的模式：BGR、HSV、LAB
    // switch (this->armor_config.PROCESSING_MODE) {
        // case BGR_MODE: {
            // cout << "+++ BGR MODOL +++" << endl;
            // this->BGRProcessing(_my_color);
            // break;
        // }
        // case HSV_MODE: {
            cout << "--- HSV MODOL ---" << endl;
            this->HSVProcessing(_my_color);

    //         break;
    //     }
    //     default: {
    //         cout << "=== DEFAULT MODOL ===" << endl;
    //         this->HSVProcessing(_my_color);

    //         break;
    //     }
    // }

#ifndef RELEASE
    // 显示各部分二值图
    if (this->armor_config.IS_SHOW_BIN_IMG == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("bin_img_color", bin_img_color);
        imshow("bin_img_gray", bin_img_gray);
    }
#endif  // !RELEASE

    // 求交集
    bitwise_and(bin_img_color, bin_img_gray, bin_img_color);

    // 膨胀处理
    morphologyEx(bin_img_color, bin_img_color, MORPH_DILATE, this->ele);

#ifndef RELEASE
    // 显示最终合成二值图
    if (this->armor_config.IS_SHOW_BIN_IMG == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("bin_img_final", bin_img_color);
    }
#endif  // !RELEASE
}

/**
 * @brief BGR color space
 * @param  _my_color        own color
 */
void RM_Armor::BGRProcessing(const int& _my_color)
{
    // 分离通道
    split(this->roi_img, this->split_img);

    // 选择颜色
    switch (_my_color) {
        case RED: {
            cout << "My color is red!" << endl;

            /* my_color为红色，则处理蓝色的情况 */
            /* 灰度图与RGB同样做蓝色处理 */
            subtract(split_img[0], split_img[2], bin_img_color);  // b-r

#ifndef RELEASE
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_BLUE:", "trackbar",
                               &this->armor_config.BLUE_ARMOR_COLOR_TH, 255, nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 BGR蓝色预处理调参面板已打开 🧐" << endl;
            }

#endif  // !RELEASE

            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.BLUE_ARMOR_GRAY_TH,
                      255, THRESH_BINARY);

            // 颜色部分
            threshold(this->bin_img_color, this->bin_img_color,
                      this->armor_config.BLUE_ARMOR_COLOR_TH, 255, THRESH_BINARY);

            break;
        }
        case BLUE: {
            cout << "My color is blue!" << endl;

            /* my_color为蓝色，则处理红色的情况 */
            /* 灰度图与RGB同样做红色处理 */
            subtract(split_img[2], split_img[0], bin_img_color);  // r-b

#ifndef RELEASE
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_COLOR_TH,
                               255, nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 BGR红色预处理调参面板已打开 🧐" << endl;
            }
#endif  // !RELEASE

            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.RED_ARMOR_GRAY_TH, 255,
                      THRESH_BINARY);

            // 颜色部分
            threshold(this->bin_img_color, this->bin_img_color,
                      this->armor_config.RED_ARMOR_COLOR_TH, 255, THRESH_BINARY);

            break;
        }
        default: {
            cout << "My color is default!" << endl;

            subtract(this->split_img[0], this->split_img[2], bin_img_color1);  // b-r
            subtract(this->split_img[2], this->split_img[0], bin_img_color2);  // r-b

#ifndef RELEASE
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_COLOR_TH,
                               255, nullptr);
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("COLOR_TH_BLUE:", "trackbar",
                               &this->armor_config.BLUE_ARMOR_COLOR_TH, 255, nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 BGR通用预处理调参面板已打开 🧐" << endl;
            }
#endif  // !RELEASE

            // 亮度部分
            this->average_th = static_cast<int>(
                (this->armor_config.RED_ARMOR_GRAY_TH + this->armor_config.BLUE_ARMOR_GRAY_TH) *
                0.5);
            threshold(this->gray_img, this->bin_img_gray, average_th, 255, THRESH_BINARY);

            // 颜色部分
            threshold(this->bin_img_color1, this->bin_img_color1,
                      this->armor_config.BLUE_ARMOR_COLOR_TH, 255, THRESH_BINARY);
            threshold(this->bin_img_color2, this->bin_img_color2,
                      this->armor_config.RED_ARMOR_COLOR_TH, 255, THRESH_BINARY);

            // 求并集
            bitwise_or(bin_img_color1, bin_img_color2, bin_img_color);

            break;
        }
    }

    split_img.clear();
    vector<Mat>(split_img).swap(split_img);  // TODO:查看容量有多大
}

/**
 * @brief HSV color space
 * @param  _my_color        own color
 */
void RM_Armor::HSVProcessing(const int& _my_color)
{
    cvtColor(this->roi_img, this->hsv_img, COLOR_BGR2HSV_FULL);

    switch (_my_color) {
        case RED:

            cout << "My color is red!" << endl;

#ifndef RELEASE
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_BLUE_MAX:", "trackbar", &this->armor_config.H_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("H_BLUE_MIN:", "trackbar", &this->armor_config.H_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("S_BLUE_MAX:", "trackbar", &this->armor_config.S_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("S_BLUE_MIN:", "trackbar", &this->armor_config.S_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("V_BLUE_MAX:", "trackbar", &this->armor_config.V_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("V_BLUE_MIN:", "trackbar", &this->armor_config.V_BLUE_MIN, 255,
                               nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 HSV蓝色预处理调参面板已打开 🧐" << endl;
            }
#endif  // !RELEASE

            // 颜色部分
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_BLUE_MIN, this->armor_config.S_BLUE_MIN,
                           this->armor_config.V_BLUE_MIN),
                    Scalar(this->armor_config.H_BLUE_MAX, this->armor_config.S_BLUE_MAX,
                           this->armor_config.V_BLUE_MAX),
                    bin_img_color);

            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.BLUE_ARMOR_GRAY_TH,
                      255, THRESH_BINARY);

            break;
        case BLUE:

            cout << "My color is blue!" << endl;

#ifndef RELEASE
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");
                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_RED_MAX:", "trackbar", &this->armor_config.H_RED_MAX, 360,
                               nullptr);
                createTrackbar("H_RED_MIN:", "trackbar", &this->armor_config.H_RED_MIN, 360,
                               nullptr);
                createTrackbar("S_RED_MAX:", "trackbar", &this->armor_config.S_RED_MAX, 255,
                               nullptr);
                createTrackbar("S_RED_MIN:", "trackbar", &this->armor_config.S_RED_MIN, 255,
                               nullptr);
                createTrackbar("V_RED_MAX:", "trackbar", &this->armor_config.V_RED_MAX, 255,
                               nullptr);
                createTrackbar("V_RED_MIN:", "trackbar", &this->armor_config.V_RED_MIN, 255,
                               nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 HSV红色预处理调参面板已打开 🧐" << endl;
            }
#endif  // !RELEASE

            // 颜色部分

            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_RED_MIN, this->armor_config.S_RED_MIN,
                           this->armor_config.V_RED_MIN),
                    Scalar(this->armor_config.H_RED_MAX, this->armor_config.S_RED_MAX,
                           this->armor_config.V_RED_MAX),
                    bin_img_color);

            // 亮度部分
            threshold(this->gray_img, this->bin_img_gray, this->armor_config.RED_ARMOR_GRAY_TH, 255,
                      THRESH_BINARY);

            break;
        default:

            cout << "My color is default!" << endl;

#ifndef RELEASE
            if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                namedWindow("trackbar");

                createTrackbar("GRAY_TH_RED:", "trackbar", &this->armor_config.RED_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_RED_MAX:", "trackbar", &this->armor_config.H_RED_MAX, 360,
                               nullptr);
                createTrackbar("H_RED_MIN:", "trackbar", &this->armor_config.H_RED_MIN, 360,
                               nullptr);
                createTrackbar("S_RED_MAX:", "trackbar", &this->armor_config.S_RED_MAX, 255,
                               nullptr);
                createTrackbar("S_RED_MIN:", "trackbar", &this->armor_config.S_RED_MIN, 255,
                               nullptr);
                createTrackbar("V_RED_MAX:", "trackbar", &this->armor_config.V_RED_MAX, 255,
                               nullptr);
                createTrackbar("V_RED_MIN:", "trackbar", &this->armor_config.V_RED_MIN, 255,
                               nullptr);

                createTrackbar("GRAY_TH_BLUE:", "trackbar", &this->armor_config.BLUE_ARMOR_GRAY_TH,
                               255, nullptr);
                createTrackbar("H_BLUE_MAX:", "trackbar", &this->armor_config.H_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("H_BLUE_MIN:", "trackbar", &this->armor_config.H_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("S_BLUE_MAX:", "trackbar", &this->armor_config.S_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("S_BLUE_MIN:", "trackbar", &this->armor_config.S_BLUE_MIN, 255,
                               nullptr);
                createTrackbar("V_BLUE_MAX:", "trackbar", &this->armor_config.V_BLUE_MAX, 255,
                               nullptr);
                createTrackbar("V_BLUE_MIN:", "trackbar", &this->armor_config.V_BLUE_MIN, 255,
                               nullptr);
                imshow("trackbar", trackbar_img);
                cout << "🧐 HSV通用预处理调参面板已打开 🧐" << endl;
            }
#endif  // !RELEASE

            // 亮度部分
            this->average_th = static_cast<int>(
                (this->armor_config.RED_ARMOR_GRAY_TH + this->armor_config.BLUE_ARMOR_GRAY_TH) *
                0.5);
            threshold(this->gray_img, this->bin_img_gray, average_th, 255, THRESH_BINARY);

            // 红色
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_RED_MIN, this->armor_config.S_RED_MIN,
                           this->armor_config.V_RED_MIN),
                    Scalar(this->armor_config.H_RED_MAX, this->armor_config.S_RED_MAX,
                           this->armor_config.V_RED_MAX),
                    bin_img_color2);
            // 蓝色
            inRange(this->hsv_img,
                    Scalar(this->armor_config.H_BLUE_MIN, this->armor_config.S_BLUE_MIN,
                           this->armor_config.V_BLUE_MIN),
                    Scalar(this->armor_config.H_BLUE_MAX, this->armor_config.S_BLUE_MAX,
                           this->armor_config.V_BLUE_MAX),
                    bin_img_color1);

            // 求并集
            bitwise_or(bin_img_color1, bin_img_color2, bin_img_color);
            break;
    }
}

/**
 * @brief 创建灯条筛选条件的滑动条
 */
void RM_Armor::createLightTrackbar()
{
#ifndef RELEASE
    // 调整阈值
    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        namedWindow("light_trackbar");

        createTrackbar("RATIO_W_H_MAX * 0.01", "light_trackbar", &h_w_ratio_max_int, 1300, nullptr);
        createTrackbar("RATIO_W_H_MIN * 0.01", "light_trackbar", &h_w_ratio_min_int, 1300, nullptr);
        createTrackbar("area", "light_trackbar", &this->armor_config.light_cfg.LIGHT_AREA, 10000,
                       nullptr);
        createTrackbar("height", "light_trackbar", &this->armor_config.light_cfg.LIGHT_HEIGHT,
                       10000, nullptr);
        createTrackbar("length_min", "light_trackbar",
                       &this->armor_config.light_cfg.LIGHT_LENGTH_MIN, 10000, nullptr);
        createTrackbar("length_max", "light_trackbar",
                       &this->armor_config.light_cfg.LIGHT_LENGTH_MAX, 10000, nullptr);
        imshow("light_trackbar", light_trackbar_img);

        this->armor_config.light_cfg.RATIO_W_H_MAX = static_cast<float>(h_w_ratio_max_int) * 0.01;
        this->armor_config.light_cfg.RATIO_W_H_MIN = static_cast<float>(h_w_ratio_min_int) * 0.01;

        cout << "🧐 灯条 框选阈值选择面板已打开 🧐" << endl;
    }
#endif  // !RELEASE
}

/**
 * @brief 创建装甲板筛选条件的滑动条
 */
void RM_Armor::createCandidateArmorTrackbar()
{
#ifndef RELEASE
    // 使用滑动条
    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        namedWindow("armor_trackbar");
        // 滑动条修改
        createTrackbar("DIFF_LIGHT_Y * 0.1 = ", "armor_trackbar", &this->diff_light_y, 100,
                       nullptr);
        createTrackbar("DISTANCE_MIN_TH = ", "armor_trackbar",
                       &this->armor_config.candidate_armor_cfg.DISTANCE_MIN_TH, 10000, nullptr);
        createTrackbar("DISTANCE_MULTIPLE_TH * 0.1 = ", "armor_trackbar",
                       &this->distance_multiple_th, 1000, nullptr);

        createTrackbar("DIFF_ANGLE_TH_SAME * 0.1 = ", "armor_trackbar", &this->diff_angle_th_same,
                       300, nullptr);

        createTrackbar("LIGHT_SIZE_RATIO_MAX * 0.1 = ", "armor_trackbar",
                       &this->light_size_ratio_max, 100, nullptr);
        createTrackbar("LIGHT_SIZE_RATIO_MIN * 0.1 = ", "armor_trackbar",
                       &this->light_size_ratio_min, 100, nullptr);

        createTrackbar("MATCHING_TH  = ", "armor_trackbar",
                       &this->armor_config.candidate_armor_cfg.MATCHING_TH, 10, nullptr);
        imshow("armor_trackbar", armor_trackbar_img);

        // 更新阈值
        this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y =
            static_cast<float>(this->diff_light_y) * 0.1;
        this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH =
            static_cast<float>(this->distance_multiple_th) * 0.1;
        this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_SAME =
            static_cast<float>(this->diff_angle_th_same) * 0.1;
        this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN =
            static_cast<float>(this->light_size_ratio_min) * 0.1;
        this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX =
            static_cast<float>(this->light_size_ratio_max) * 0.1;

        cout << "🧐 装甲板 阈值调整面板已打开 🧐" << endl;
    }
#endif  // !RELEASE
}

/**
 * @brief 筛选灯条
 * @return true             搜索成功
 * @return false            搜索失败
 */
bool RM_Armor::lightScreenting()
{
    // 查找轮廓
    findContours(this->bin_img_color, this->contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    // 灯条数量过少直接退出
    if (this->contours.size() < 2) {
#ifdef DEBUG
        cout << "可筛灯条数量过少 --->" << endl;
        //清空内存
        this->contours.clear();
        vector<vector<Point>>(contours).swap(contours);
        return false;
#else
        //清空内存
        this->contours.clear();
        vector<vector<Point>>(contours).swap(contours);

        this->is_last_data_catch = false;
        // this->is_Lost_target     = true;
        this->lose_target_cnt += 1;
#endif  // DEBUG
    }

    // 遍历灯条
    for (size_t i = 0; i < contours.size(); ++i) {
        // 排查size小于6的
        if (this->contours[i].size() < 6) {
            continue;
        }

        // 内接椭圆
        this->rect_tmp = fitEllipse(contours[i]);

        // 准备各种条件所需变量：宽、高、面积、角度
        this->light.inputparam(this->rect_tmp, i, contours[i], this->armor_roi);

        // 画出全部可选灯条
        // Point2f vtx[4];
        // this->rect_tmp.points(vtx);
        // for (int j = 0; j < 4; ++j) {
        //     line(src_img, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 255), 2, 8, 0);
        // }

#ifdef DEBUG
        // 根据范围选择灯条 TODO
        if (this->light.returnDistToCenter() > this->armor_range) {
            continue;
        }
#endif  // DEBUG

        // 条件判断：角度、宽高比、面积、周长、高度
        if ((this->light.returnAspectRatio() > this->armor_config.light_cfg.RATIO_W_H_MIN &&
             this->light.returnAspectRatio() < this->armor_config.light_cfg.RATIO_W_H_MAX) &&
            (this->light.returnAngle() >= 55 && this->light.returnAngle() <= 125) &&
            (this->light.returnRect().size.area() > this->armor_config.light_cfg.LIGHT_AREA) &&
            (this->light.returnPerimeter() < this->armor_config.light_cfg.LIGHT_LENGTH_MAX &&
             this->light.returnPerimeter() > this->armor_config.light_cfg.LIGHT_LENGTH_MIN) &&
            this->light.returnHeight() > this->armor_config.light_cfg.LIGHT_HEIGHT) {
            // 正确：更新数据、存入容器、画出灯条，显示数据
            this->light_box.emplace_back(light);

#ifndef RELEASE
            // 画出正确灯条，并显示数据
            // 调参时需要显示的灯条图像
            if (this->armor_config.IS_DRAW_LIGHT == 1 &&
                this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                // 高度
                Point put_height = Point(this->light.returnRect().center.x,
                                         this->light.returnRect().center.y - 60);
                putText(light_img, to_string(this->light.returnHeight()), put_height,
                        FONT_HERSHEY_PLAIN, 1, Scalar(127, 255, 0), 1, 8, false);
                // 周长
                Point put_perimeter = Point(this->light.returnRect().center.x,
                                            this->light.returnRect().center.y - 50);
                putText(light_img, to_string(this->light.returnPerimeter()), put_perimeter,
                        FONT_HERSHEY_PLAIN, 1, Scalar(127, 255, 0), 1, 8, false);
                // 角度
                Point put_angle = Point(this->light.returnRect().center.x,
                                        this->light.returnRect().center.y - 40);
                putText(light_img, to_string(this->light.returnAngle()), put_angle,
                        FONT_HERSHEY_PLAIN, 1, Scalar(255, 245, 0), 1, 8, false);
                // 索引
                Point put_index = Point(this->light.returnRect().center.x,
                                        this->light.returnRect().center.y - 30);
                putText(light_img, to_string(this->light.returnIndex()), put_index,
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
                // 高宽比
                Point put_hw_ratio = Point(this->light.returnRect().center.x,
                                           this->light.returnRect().center.y - 20);
                putText(light_img, to_string(this->light.returnAspectRatio()), put_hw_ratio,
                        FONT_HERSHEY_PLAIN, 1, Scalar(180, 105, 255), 1, 8, false);
                // 面积
                Point put_area = Point(this->light.returnRect().center.x,
                                       this->light.returnRect().center.y - 10);
                putText(light_img, to_string(this->light.returnRect().size.area()), put_area,
                        FONT_HERSHEY_PLAIN, 1, Scalar(127, 255, 0), 1, 8, false);

                // 画出灯条
                for (int j = 0; j < 4; ++j) {
                    line(light_img, this->light.returnVertex(j),
                         this->light.returnVertex((j + 1) % 4), Scalar(0, 255, 255), 2, 8, 0);
                }
                cout << "🧐 灯条 框选已打开 🧐" << endl;
            }
#endif  // !RELEASE
        }
        // 错误：遍历下一个
    }

    //清空内存
    this->contours.clear();
    vector<vector<Point>>(contours).swap(contours);

    // 筛选得到的灯条小于二，则返回错误并退出
    if (light_box.size() < 2) {
#ifdef DEBUG
        cout << "有效灯条数量小于二 --->" << endl;
#else
        this->is_last_data_catch = false;
        // this->is_Lost_target     = true;
        this->lose_target_cnt += 1;
#endif  // DEBUG
        return false;
    }

    return true;
}

/**
 * @brief 筛选候选装甲板
 * @return true             搜索成功
 * @return false            搜索失败
 */
bool RM_Armor::armorScreenting()
{
    cout << "灯条数量：" << this->light_box.size() << endl;

    // 更新标志位 TODO
    this->is_last_data_catch = false;

#ifndef RELEASE
    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1 && this->armor_config.IS_DRAW_ARMOR_RECT == 1) {
        // 显示灯条数目
        Point put_size = Point(60, 10);
        putText(this->armor_img, to_string(this->light_box.size()), put_size, FONT_HERSHEY_PLAIN, 1,
                Scalar(0, 255, 155), 1, 8, false);
    }
#endif  // !RELEASE

    // 排序，按照x值从左往右进行排序
    sort(this->light_box.begin(), this->light_box.end(), [](const Light& l1, const Light& l2) {
        return l1.returnRect().center.x < l2.returnRect().center.x;
    });

    // TODO:加条件控制显示
    // 更新索引号
    int i = 0;
    for (auto iter = light_box.begin(); iter != light_box.end(); ++iter) {
        (*iter).updateIndex(i);
        i += 1;
        // cout << "center = " << (*iter).center << endl;

#ifndef RELEASE
        // 显示排序结果
        if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 &&
            this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
            // 新索引号
            Point put_index =
                Point((*iter).returnRect().center.x, (*iter).returnRect().center.y - 30);
            putText(this->armor_img, to_string((*iter).returnIndex()), put_index,
                    FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
            cout << "🧐 灯条 排序后编号已打开 🧐" << endl;
        }
#endif  // !RELEASE
    }

    // 双指针循环匹配
    this->ra          = this->light_box.size() - 1;
    this->la          = ra - 1;
    this->armor_index = 0;  // index
    this->over_cnt    = 0;

#ifndef RELEASE
    // 使用滑动条
    if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        Point put_diff_light_y = Point(10, 20);
        putText(armor_img, to_string(this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y),
                put_diff_light_y, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_distance_multiple_th = Point(10, 30);
        putText(armor_img, to_string(this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH),
                put_distance_multiple_th, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_armor_type_th = Point(10, 50);
        putText(armor_img, to_string(this->armor_config.candidate_armor_cfg.ARMOR_TYPE_TH),
                put_armor_type_th, FONT_HERSHEY_PLAIN, 1, Scalar(155, 255, 0), 1, 8, false);
    }
#endif  // !RELEASE

    // 重置标志位
    this->is_include_armor = false;

    while (ra > 0) {
        // 准备匹配的条件计算
        this->light_right = this->light_box[ra];
        this->light_left  = this->light_box[la];
        cout << "ra = " << ra << " la = " << la << endl;

        // 条件一：灯条高度差
        this->light_y_diff =
            fabs(this->light_left.returnRect().center.y - this->light_right.returnRect().center.y);
        this->is_height_diff_catch =
            (this->light_y_diff <
             (this->light_left.returnHeight() + this->light_right.returnHeight()) *
                 this->armor_config.candidate_armor_cfg.DIFF_LIGHT_Y);

        // 条件二：灯条的间距 貌似没什么用？
        this->light_distance = centerDistance(this->light_left.returnRect().center,
                                              this->light_right.returnRect().center);
        this->light_height_max =
            MAX(this->light_left.returnHeight(), this->light_right.returnHeight());
        this->light_height_min =
            MIN(this->light_left.returnHeight(), this->light_right.returnHeight());
        this->is_light_distance_catch =
            (this->light_distance <
                 this->light_height_max *
                     this->armor_config.candidate_armor_cfg.DISTANCE_MULTIPLE_TH &&
             this->light_distance > this->armor_config.candidate_armor_cfg.DISTANCE_MIN_TH);

        // 条件三：灯条高度比例
        this->is_height_ratio = (this->light_height_max / this->light_height_min) < 1.5;

        // 条件四：灯条大小比例
        this->light_size_ratio = light_distance / MIN(this->light_left.returnRect().size.height,
                                                      this->light_right.returnRect().size.height);
        this->is_similar_size =
            this->light_size_ratio < this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MAX &&
            this->light_size_ratio > this->armor_config.candidate_armor_cfg.LIGHT_SIZE_RATIO_MIN;

        // 条件五：灯条的角度差：可调
        // 灯条的内外八:兵种按照相应的种类进行选取即可，这里全部列出
        this->is_light_angle_catch = false;
        this->is_light_angle_catch =
            (fabs(this->light_left.returnAngle() - this->light_right.returnAngle()) <=
             this->armor_config.candidate_armor_cfg.DIFF_ANGLE_TH_SAME);

#ifndef RELEASE
        // Debug 绝对不能动
        // 强制关闭条件开关
        if (this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
            namedWindow("armor_trackbar");
            createTrackbar("is_enable_light_y_diff", "armor_trackbar", &is_enable_diff_light_y, 1,
                           nullptr);
            createTrackbar("is_enable_distance_multiple_th", "armor_trackbar",
                           &is_enable_distance_multiple_th, 1, nullptr);
            createTrackbar("is_enable_diff_angle_th", "armor_trackbar", &is_enable_diff_angle_th, 1,
                           nullptr);
            createTrackbar("is_enable_similar_size", "armor_trackbar", &is_enable_similar_size, 1,
                           nullptr);
            createTrackbar("is_height_ratio", "armor_trackbar", &is_enable_height_ratio, 1,
                           nullptr);

            if (is_enable_diff_angle_th == 1) {
                this->is_light_angle_catch = true;
            }
            if (is_enable_diff_light_y == 1) {
                this->is_height_diff_catch = true;
            }
            if (is_enable_distance_multiple_th == 1) {
                this->is_light_distance_catch = true;
            }
            if (is_enable_similar_size == 1) {
                this->is_similar_size = true;
            }
            if (is_enable_height_ratio == 1) {
                this->is_height_ratio = true;
            }
            cout << "🧐 装甲板 条件选择使能面板已打开 🧐" << endl;
        }
#endif  // !RELEASE

        // 匹配条件的筛选 一个个来
        if (is_height_diff_catch && is_light_distance_catch && is_light_angle_catch &&
            is_similar_size && is_height_ratio) {
            // 匹配成功则滑动窗口
            ra = la - 1;

            // 若匹配成功之前有跳过的灯条，则从跳过灯条开始继续匹配
            if (this->is_include_armor) {
                ra               = include_armor_index;
                is_include_armor = false;
            }

            la = ra - 1;

            // 更新装甲板数据 i
            // 读取大小装甲阈值
            this->armor.inputparam(this->light_left, this->light_right, armor_index,
                                   this->armor_roi);
            this->armor_box.emplace_back(this->armor);

#ifndef RELEASE
            // 画图
            if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 &&
                this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                for (int j = 0; j < 4; ++j) {
                    line(this->armor_img, this->armor.returnVertex(j),
                         this->armor.returnVertex((j + 1) % 4), Scalar(150, 200, 0), 2, 8, 0);
                }

                Point put_armor_angle = Point(this->armor.returnRect().center.x,
                                              this->armor.returnRect().center.y - 20);
                putText(armor_img, to_string(this->armor.returnAngle()), put_armor_angle,
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);

                Point put_size = Point(this->armor.returnRect().center.x,
                                       this->armor.returnRect().center.y - 30);
                switch (this->armor.returnArmorType()) {
                    case SMALL_ARMOR:
                        putText(armor_img, "Small", put_size, FONT_HERSHEY_PLAIN, 1,
                                Scalar(0, 255, 0), 1, 8, false);
                        break;
                    case BIG_ARMOR:
                        putText(armor_img, "Big", put_size, FONT_HERSHEY_PLAIN, 1,
                                Scalar(0, 255, 0), 1, 8, false);
                        break;
                    default:
                        putText(armor_img, "Small", put_size, FONT_HERSHEY_PLAIN, 1,
                                Scalar(0, 255, 0), 1, 8, false);
                        break;
                }

                Point put_aspect_ratio = Point(this->armor.returnRect().center.x,
                                               this->armor.returnRect().center.y - 50);
                putText(armor_img, to_string(this->armor.returnAspectRatio()), put_aspect_ratio,
                        FONT_HERSHEY_PLAIN, 1, Scalar(120, 255, 0), 1, 8, false);
            }
#endif  // !RELEASE

            armor_index += 1;
            over_cnt = 0;
        }
        else {
            // 匹配失败继续匹配下一个
            over_cnt += 1;
            if (over_cnt == 1) {
                include_armor_index = la;
                is_include_armor    = true;
            }
            --la;
        }

        //设置范围，根据参数设定
        // 判断范围是否超出参数设定
        // 匹配成功 貌似不用管

        if (over_cnt != 0) {
            // 匹配不成功
            if (over_cnt > this->armor_config.candidate_armor_cfg.MATCHING_TH) {
                // 失败次数大于阈值
                --ra;
                la               = ra - 1;
                over_cnt         = 0;
                is_include_armor = false;
            }
            else if (over_cnt <= this->armor_config.candidate_armor_cfg.MATCHING_TH) {
                // 失败次数小于阈值
                if (light_y_diff > light_right.returnHeight() * 0.5 && la < 0) {
                    // 高度差大于阈值
                    --ra;
                    la               = ra - 1;
                    over_cnt         = 0;
                    is_include_armor = false;
                }
                else if (light_y_diff <= light_right.returnHeight() * 0.5) {
                    // 高度差小于阈值
                    --ra;
                    la               = ra - 1;
                    over_cnt         = 0;
                    is_include_armor = false;
                }
            }
        }
    }

    this->light_box.clear();
    vector<Light>(this->light_box).swap(this->light_box);

    if (this->armor_box.size() < 1) {
#ifdef DEBUG
        // 无目标，更新标志位为false，计数器++，退出，重置数据
        this->is_last_data_catch = false;
        // this->is_Lost_target     = true;  // 丢失目标
        this->lose_target_cnt += 1;
#endif  // DEBUG
        cout << "正确装甲板数目为零 --->" << endl;
        return false;
    }
#ifdef DEBUG
    // 有目标，更新标志位为true，计数器清空，重置数据
    this->is_last_data_catch = true;  // 检测到装甲板，则下一帧会标识上一帧有数据
    this->is_Lost_target     = false;  // 未丢失目标
    this->lose_target_cnt    = 0;
#endif  // DEBUG
    return true;
}

/**
 * @brief 选取最优装甲板
 */
bool RM_Armor::armorSelectOptimum()
{
    cout << "装甲板数量：" << this->armor_box.size() << endl;

    if (this->armor_box.size() == 1) {
        cout << "只有一个目标" << endl;

        // 若只有一个，直接当做最优解，提前退出
        this->final_armor = armor_box.back();
    }
    else {
        cout << "有多个目标" << endl;
        // 多个的时候根据两个方法进行筛选test：TODO
        // 方法一：先按照中心距进行排序，再根据一定斜率阈值进行选择
        // 方法二：先根据斜率进行排序，再根据中心点距离逐个排查斜率的排序按照一定条件来进行最终的选择

        // 循环遍历armor_box ，先根据距离进行排序方法一：离中心点越近，数字越小
        sort(this->armor_box.begin(), this->armor_box.end(),
             [](CandidateArmor& c1, CandidateArmor& c2) {
                 return c1.returnDistToCenter() <
                        c2.returnDistToCenter();  // 按中心距进行排序，越小越前
             });

        int i                    = 0;
        this->aspest_ratio_min   = 9999.f;
        this->dist_to_center_min = 9999999999.f;
        // 中点可以通过candidatearmor来读 TODO
        for (auto iter = armor_box.begin(); iter != armor_box.end(); ++iter) {
            (*iter).updateIndex(i);

#ifndef RELEASE
            //显示排序结果
            if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 &&
                this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
                Point put_index =
                    Point((*iter).returnRect().center.x, (*iter).returnRect().center.y - 30);
                putText(this->dst_img, to_string((*iter).returnIndex()), put_index,
                        FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
                cout << "🧐 装甲板 排序后编号已打开 🧐" << endl;
            }
#endif  // !RELEASE

            // TODO 加上roi的左上角距离
            // if ((*iter).returnDistToCenter() <= this->dist_to_center_min) {
            //     this->dist_to_center_min = (*iter).returnDistToCenter();
            if (fabs((*iter).returnAngle() - 180.f) <= 8.f) {
                // this->aspest_ratio_min =
                //     fabs((*iter).returnAngle() - 180.f);  //待确定是否正确 TODO

                this->final_armor = (*iter);
                break;
            }
            if (i == 2) {
                break;
            }

            i += 1;
        }
        // 得到最终值，退出循环
    }

    this->last_armor = this->final_armor;

#ifndef RELEASE
    if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        Point2f vtx[4];
        this->final_armor.returnRect().points(vtx);
        for (int j = 0; j < 4; ++j) {
#    ifdef DEBUG

            line(this->src_img, this->final_armor.returnVertex(j),
                 this->final_armor.returnVertex((j + 1) % 4), Scalar(0, 255, 0), 2, 8, 0);
#    endif  // DEBUG
            line(this->dst_img, this->final_armor.returnVertex(j),
                 this->final_armor.returnVertex((j + 1) % 4), Scalar(0, 255, 0), 2, 8, 0);
        }
#    ifdef DEBUG

        circle(dst_img, Point(640, 450), (armor_range), Scalar(255, 255, 255), 1, 8, 0);

        Point put_first_cnt = Point(100, 50);
        putText(this->src_img, to_string(this->armor_config.FIRST_LOSE_CNT_MAX), put_first_cnt,
                FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_second_cnt = Point(100, 60);
        putText(this->src_img, to_string(this->armor_config.SECOND_LOSE_CNT_MAX), put_second_cnt,
                FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_first_range = Point(100, 70);
        putText(this->src_img, to_string(this->armor_config.FIRST_LEVEL_LOSS_RANGE),
                put_first_range, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
        Point put_second_range = Point(100, 80);
        putText(this->src_img, to_string(this->armor_config.SECOND_LEVEL_LOSS_RANGE),
                put_second_range, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
#    endif  // DEBUG
        // 对最终目标进行顶点排序
    }
#endif  // !RELEASE

    this->sortArmorVertex();

#ifndef RELEASE
    // 显示装甲板图像
    if (this->armor_config.IS_DRAW_ARMOR_RECT == 1 && this->armor_config.IS_PARAM_ADJUSTMENT == 1) {
        imshow("final_armor", this->dst_img);
        cout << "🧐 最优装甲板 框选图像已打开 🧐" << endl;
    }
#endif  // !RELEASE

#ifdef DEBUG
    // 有目标，更新标志位为true，计数器清空，重置数据
    this->is_last_data_catch = true;  // 检测到装甲板，则下一帧会标识上一帧有数据
    this->is_Lost_target     = false;  // 未丢失目标
    this->lose_target_cnt    = 0;
#endif // DEBUG

    this->armor_box.clear();
    vector<CandidateArmor>(this->armor_box).swap(this->armor_box);

    return true;
}

#ifndef RELEASE
/**
 * @brief 创建最优装甲板滑动条
 */
void RM_Armor::createOpeimumArmorTrackbar()
{
    namedWindow("OptimumArmor_trackbar");
    createTrackbar("FIRST_LOSE_CNT_MAX = ", "OptimumArmor_trackbar",
                   &this->armor_config.FIRST_LOSE_CNT_MAX, 1000, nullptr);
    createTrackbar("SECOND_LOSE_CNT_MAX = ", "OptimumArmor_trackbar",
                   &this->armor_config.SECOND_LOSE_CNT_MAX, 1000, nullptr);
    createTrackbar("FIRST_LEVEL_LOSS_RANGE = ", "OptimumArmor_trackbar",
                   &this->armor_config.FIRST_LEVEL_LOSS_RANGE, 1000, nullptr);
    createTrackbar("SECOND_LEVEL_LOSS_RANGE = ", "OptimumArmor_trackbar",
                   &this->armor_config.SECOND_LEVEL_LOSS_RANGE, 1000, nullptr);
    imshow("OptimumArmor_trackbar", optimum_armor_trackbar_img);

    cout << "🧐 最优装甲板 阈值选择面板已打开 🧐" << endl;
}
#endif  // !RELEASE

/**
 * @brief 装甲板顶点排序
 */
void RM_Armor::sortArmorVertex()
{
    Point2f lu, ld, ru, rd;

    // 二、拟合灯条外顶点

    sort(this->final_armor.returnLeftLight().returnVertex(),
         this->final_armor.returnLeftLight().returnVertex() + 4,
         [](const Point2f& p1, const Point2f& p2) { return p1.y < p2.y; });
    sort(this->final_armor.returnRightLight().returnVertex(),
         this->final_armor.returnRightLight().returnVertex() + 4,
         [](const Point2f& p1, const Point2f& p2) { return p1.y < p2.y; });

    if (this->final_armor.returnLeftLight().returnVertex(0).x <
        this->final_armor.returnLeftLight().returnVertex(1).x) {
        lu = this->final_armor.returnLeftLight().returnVertex(0);
        ld = this->final_armor.returnLeftLight().returnVertex(2);
    }
    else {
        lu = this->final_armor.returnLeftLight().returnVertex(1);
        ld = this->final_armor.returnLeftLight().returnVertex(3);
    }
    if (this->final_armor.returnRightLight().returnVertex(2).y <
        this->final_armor.returnRightLight().returnVertex(3).y) {
        ru = this->final_armor.returnRightLight().returnVertex(1);
        rd = this->final_armor.returnRightLight().returnVertex(3);
    }
    else {
        ru = this->final_armor.returnRightLight().returnVertex(0);
        rd = this->final_armor.returnRightLight().returnVertex(2);
    }

    // lu += this->armor_config.CAMERA_RESOLUTION_FOV;
    // ld += this->armor_config.CAMERA_RESOLUTION_FOV;
    // ru += this->armor_config.CAMERA_RESOLUTION_FOV;
    // rd += this->armor_config.CAMERA_RESOLUTION_FOV;

    // 一 拟合装甲板顶点
    // Point2f vertex[4];
    // this->armor.returnRect().points(vertex);

    // sort(vertex, vertex + 4, [](const Point2f& p1, const Point2f& p2) { return p1.x < p2.x;
    // });

    // if (vertex[0].y < vertex[1].y){
    //     lu = vertex[0];
    //     ld = vertex[1];
    // }
    // else{
    //     lu = vertex[1];
    //     ld = vertex[0];
    // }
    // if (vertex[2].y < vertex[3].y)	{
    //     ru = vertex[2];
    //     rd = vertex[3];
    // }
    // else {
    //     ru = vertex[3];
    //     rd = vertex[2];
    // }

#ifndef RELEASE
    Point put_lu = Point(lu.x, lu.y - this->armor_config.CAMERA_RESOLUTION_FOV.y);
    putText(dst_img, "0", put_lu, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
    Point put_ru = Point(ru.x, ru.y - this->armor_config.CAMERA_RESOLUTION_FOV.y);
    putText(dst_img, "1", put_ru, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
    Point put_rd = Point(rd.x, rd.y - this->armor_config.CAMERA_RESOLUTION_FOV.y);
    putText(dst_img, "2", put_rd, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
    Point put_ld = Point(ld.x, ld.y - this->armor_config.CAMERA_RESOLUTION_FOV.y);
    putText(dst_img, "3", put_ld, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
#endif  // !RELEASE

    // 二 左右灯条外顶点

    target_2d.clear();
    target_2d.emplace_back(lu);
    target_2d.emplace_back(ru);
    target_2d.emplace_back(rd);
    target_2d.emplace_back(ld);
}

/**
 * @brief 返回装甲板四个顶点
 * @return vector<Point2f>
 */
vector<Point2f> RM_Armor::returnTarget2DPoint()
{
    return this->target_2d;
}

/**
 * @brief 返回最优装甲板
 * @return CandidateArmor&
 */
CandidateArmor& RM_Armor::returnFinalArmor()
{
    return this->final_armor;
}

bool RM_Armor::returnIsLastData(){
    return this->is_last_data_catch;
}
/**
 * @brief 计算两点距离
 * @param  p1               第一个点
 * @param  p2               第二个点
 * @return float            返回两点距离
 */
float centerDistance(const Point& p1, const Point& p2)
{
    float D =
        static_cast<float>(sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y))));
    return D;
}

void RM_Armor::ArmorVertex(int a)
{
    target_2d[0].x +=a;
    target_2d[1].x +=a;
    target_2d[2].x +=a;
    target_2d[3].x +=a;
}