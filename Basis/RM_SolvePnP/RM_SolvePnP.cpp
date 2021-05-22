#include "Basis/RM_SolvePnP/RM_SolvePnP.h"
/**
 * @brief Construct a new rm solvepnp::rm solvepnp object
 * @param  _pnp_config      pnp参数结构体
 */
RM_SolvePnP::RM_SolvePnP(const PNP_Cfg& _pnp_config) : pnp_cfg(_pnp_config)
{
    this->big_armor_3d.reserve(4);
    this->small_armor_3d.reserve(4);
    this->buff_armor_3d.reserve(4);
    // this->target_2d.reserve(4);

    this->reference_Img.reserve(4);
    this->reference_Obj.reserve(4);

    // 读取摄像头标定xml文件
    cv::FileStorage fs(this->pnp_cfg.CAMERA_PARAM_FILE, cv::FileStorage::READ);

    // 读取相机内参和畸变矩阵
    fs["camera-matrix"] >> this->camera_matrix;
    fs["distortion"] >> this->distortion_coeff;
    if (this->camera_matrix.empty() || this->distortion_coeff.empty()) {
        cout << "❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌ ❌" << endl;
    }

    this->offset_armor_pitch  = this->pnp_cfg.OFFSET_ARMOR_PITCH * 10;
    this->offset_armor_yaw    = this->pnp_cfg.OFFSET_ARMOR_YAW * 10;
    this->offset_pitch_symbol = this->pnp_cfg.OFFSET_PITCH_SYMBOL;
    this->offset_yaw_symbol   = this->pnp_cfg.OFFSET_YAW_SYMBOL;

    std::cout << this->camera_matrix << std::endl;
    std::cout << this->distortion_coeff << std::endl;

    std::cout << "RM_SolvePnP is readied" << std::endl;
}

/**
 * @brief   角度解算
 * @param  _messenger       串口数据发送包
 * @param  _point_2d        装甲板各顶点
 * @param  _type            目标类型
 * @param  _bullet_speed    弹速
 * @param  _src_img         画板
 * @param  _buff_angle      目标扇叶角度
 */
void RM_SolvePnP::runSolvePnP(const std::vector<cv::Point2f>& _point_2d,
                              const armor_size&               _type,
                              cv::Mat&                        _src_img)
{
    // 更新装甲板各顶点

    // 切换目标模型
    // pnp计算
    switch (_type) {
        case BIG_ARMOR:
            cv::solvePnP(big_armor_3d, _point_2d, camera_matrix, distortion_coeff,
                         position_in_camera_rvec, position_in_camera, false,
                         cv::SOLVEPNP_ITERATIVE);
            break;
        case SMALL_ARMOR:
            cv::solvePnP(small_armor_3d, _point_2d, camera_matrix, distortion_coeff,
                         position_in_camera_rvec, position_in_camera, false,
                         cv::SOLVEPNP_ITERATIVE);
            break;
        case BUFF_ARMOR:
            cv::solvePnP(buff_armor_3d, _point_2d, camera_matrix, distortion_coeff,
                         position_in_camera_rvec, position_in_camera, false,
                         cv::SOLVEPNP_ITERATIVE);
            break;

        default:
            cv::solvePnP(small_armor_3d, _point_2d, camera_matrix, distortion_coeff,
                         position_in_camera_rvec, position_in_camera, false,
                         cv::SOLVEPNP_ITERATIVE);
            break;
    }

#ifndef RELEASE
    if (this->pnp_cfg.IS_DRAW_COORDINATE == 1) {
        // 画图
        drawCoordinate(_src_img);
    }
#endif  // !RELEASE
}

/**
 * @brief 设置相机云台变换关系
 */
void RM_SolvePnP::setRelationPoseCameraPTZ()
{
    double theta = 0;

    theta = -atan(static_cast<double>(this->pnp_cfg.CAMERA_PTZ_Y + this->pnp_cfg.PTZ_BARREL_Y)) /
            static_cast<double>(overlap_dist);

    double r_data[] = {1, 0, 0, 0, cos(theta), sin(theta), 0, -sin(theta), cos(theta)};
    double t_data[] = {static_cast<double>(this->pnp_cfg.CAMERA_PTZ_X),
                       static_cast<double>(this->pnp_cfg.CAMERA_PTZ_Y),
                       static_cast<double>(this->pnp_cfg.CAMERA_PTZ_Z)};

    this->rot_camera2ptz   = cv::Mat(3, 3, CV_64FC1, r_data);
    this->trans_camera2ptz = cv::Mat(3, 1, CV_64FC1, t_data);
}

/**
 * @brief 初始化各目标的3d模型顶点
 */
void RM_SolvePnP::set3dPoint()
{
    // 小装甲板
    float half_small_x = this->pnp_cfg.SMALL_ARMOR_SIZE_W * 0.5;
    float half_small_y = this->pnp_cfg.SMALL_ARMOR_SIZE_H * 0.5;

    this->small_armor_3d.emplace_back(cv::Point3f(-half_small_x, -half_small_y, 0));
    this->small_armor_3d.emplace_back(cv::Point3f(half_small_x, -half_small_y, 0));
    this->small_armor_3d.emplace_back(cv::Point3f(half_small_x, half_small_y, 0));
    this->small_armor_3d.emplace_back(cv::Point3f(-half_small_x, half_small_y, 0));

    // 大装甲板
    float half_big_x = this->pnp_cfg.BIG_ARMOR_SIZE_W * 0.5;
    float half_big_y = this->pnp_cfg.BIG_ARMOR_SIZE_H * 0.5;

    this->big_armor_3d.emplace_back(cv::Point3f(-half_big_x, -half_big_y, 0));
    this->big_armor_3d.emplace_back(cv::Point3f(half_big_x, -half_big_y, 0));
    this->big_armor_3d.emplace_back(cv::Point3f(half_big_x, half_big_y, 0));
    this->big_armor_3d.emplace_back(cv::Point3f(-half_big_x, half_big_y, 0));

    // 能量机关扇叶装甲板
    float half_buff_x = this->pnp_cfg.BUFF_ARMOR_SIZE_W * 0.5;
    float half_buff_y = this->pnp_cfg.BUFF_ARMOR_SIZE_H * 0.5;

    this->buff_armor_3d.emplace_back(cv::Point3f(-half_buff_x, -half_buff_y, 0));
    this->buff_armor_3d.emplace_back(cv::Point3f(half_buff_x, -half_buff_y, 0));
    this->buff_armor_3d.emplace_back(cv::Point3f(half_buff_x, half_buff_y, 0));
    this->buff_armor_3d.emplace_back(cv::Point3f(-half_buff_x, half_buff_y, 0));
}

/**
 * @brief 转换坐标系（相机->云台）
 * @param  _pos_in_camera   输入坐标系
 * @param  _transed_pos     输出坐标系
 */
void RM_SolvePnP::transformCamera2PTZ(const cv::Mat& _pos_in_camera, cv::Mat& _transed_pos)
{
    // _transed_pos = this->rot_camera2ptz * _pos_in_camera - this->trans_camera2ptz;
    _transed_pos = /* this->rot_camera2ptz * */ _pos_in_camera - this->trans_camera2ptz;  // 无旋转
}

/**
 * @brief 修复深度信息
 * @param  _depth           My Param doc
 */
void RM_SolvePnP::repairDepthInfomation(float& _depth) {}

/**
 * @brief 获取装甲板角度信息
 * @param  _messenger       串口发送信使
 */
void RM_SolvePnP::getAngle(RM_Messenger* _messenger, float yaw)
{
    // 判断深度信息的有效性，若无效，则退出
    // 修正z轴数据（深度）(模型或补偿系数)TODO
    // cam->ptz坐标系转换
    transformCamera2PTZ(position_in_camera, position_in_ptz);

    const double* _xyz = (const double*)this->position_in_ptz.data;

    this->down_t         = 0.0;
    this->offset_gravity = 0.0;
    if (_messenger->getReceiveInfo().bullet_volacity > 10e-3) {
        down_t = _xyz[2] * 0.001 / _messenger->getReceiveInfo().bullet_volacity;  // s
    }
    this->offset_gravity = 0.5 * 9.8 * down_t * down_t;
    // TODO:修复深度信息

    double xyz[3] = {_xyz[0], _xyz[1] - offset_gravity * 1000, _xyz[2]};
    // cout << "x:" << xyz[0] << "   y:" << xyz[1] << "   z:" << xyz[2] << endl;

    if (this->pnp_cfg.PTZ_BARREL_Y != 0.f) {
        this->alpha = 0.0;
        this->beta  = 0.0;
        this->alpha = asin(static_cast<double>(this->pnp_cfg.PTZ_BARREL_Y) /
                           sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2]));
        if (xyz[1] < 0) {
            beta = atan(-xyz[1] / xyz[2]);
            _messenger->getSendInfo().angle_pitch =
                static_cast<float>(-(alpha + beta));  // camera coordinate
        }
        else if (xyz[1] < static_cast<double>(this->pnp_cfg.PTZ_BARREL_Y)) {
            beta = atan(xyz[1] / xyz[2]);
            _messenger->getSendInfo().angle_pitch =
                static_cast<float>(-(alpha - beta));  // camera coordinate
        }
        else {
            beta = atan(xyz[1] / xyz[2]);
            _messenger->getSendInfo().angle_pitch =
                static_cast<float>((beta - alpha));  // camera coordinate
        }
    }
    else {
        _messenger->getSendInfo().angle_pitch = static_cast<float>(atan2(xyz[1], xyz[2]));
    }

    if (this->pnp_cfg.PTZ_BARREL_X != 0.f) {
        this->alpha = 0.0;
        this->beta  = 0.0;
        this->alpha = asin(static_cast<double>(this->pnp_cfg.PTZ_BARREL_X) /
                           sqrt(xyz[0] * xyz[0] + xyz[2] * xyz[2]));
        if (xyz[0] > 0) {
            beta = atan(-xyz[0] / xyz[2]);
            _messenger->getSendInfo().angle_yaw =
                static_cast<float>(-(alpha + beta));  // camera coordinate
        }
        else if (xyz[0] < static_cast<double>(this->pnp_cfg.PTZ_BARREL_Y)) {
            beta = atan(xyz[0] / xyz[2]);
            _messenger->getSendInfo().angle_yaw =
                static_cast<float>(-(alpha - beta));  // camera coordinate
        }
        else {
            beta = atan(xyz[0] / xyz[2]);
            _messenger->getSendInfo().angle_yaw =
                static_cast<float>((beta - alpha));  // camera coordinate
        }
    }
    else {
        _messenger->getSendInfo().angle_yaw = static_cast<float>(atan2(xyz[0], xyz[2]));
    }
    // Depth
    _messenger->getSendInfo().depth = static_cast<float>(xyz[2]);
    int dist = _messenger->getSendInfo().depth = static_cast<float>(xyz[2]);
    // Yaw
    // float ddd = 1+(dist*0.001/_messenger->getReceiveInfo().bullet_volacity);
    _messenger->getSendInfo().angle_yaw = _messenger->getSendInfo().angle_yaw * 180 / CV_PI;;

    // Pitch
    _messenger->getSendInfo().angle_pitch = _messenger->getSendInfo().angle_pitch * 180 / CV_PI;
    _messenger->getSendInfo().angle_pitch -=
        getPitch(xyz, xyz[2], xyz[1],
                 _messenger->getReceiveInfo().bullet_volacity * 1000.0);  //这里需要再减去一度

    

    this->offsetManually(_messenger);  // TODO 为读入数据，不知道为什么，

    // 更新符号信息
    _messenger->setSymbol();

    if (this->pnp_cfg.SHOW_PNP_INFORMATION == 1) {
        std::cout << "yaw:" << _messenger->getSendInfo().angle_yaw
                  << "  pitch:" << _messenger->getSendInfo().angle_pitch
                  << " depth:" << _messenger->getSendInfo().depth << std::endl;
    }
}

/**
 * @brief 获取能量机关角度信息
 * @param  _messenger       串口发送信使
 * @param  _buff_angle      能量机关目标扇叶角度
 */
void RM_SolvePnP::getAngle_buff(RM_Messenger* _messenger, const float _buff_angle)
{
    // 建立模型
    this->delta_h  = this->pnp_cfg.BUFF_GROUND_H - this->pnp_cfg.ROBOT_H - this->pnp_cfg.UPLAND_H;
    this->buff_h   = 800 * sin(_buff_angle * CV_PI / 180) + 800;
    this->target_h = this->delta_h + this->buff_h;
    this->distance = sqrt(this->target_h * this->target_h +
                          this->pnp_cfg.BUFF_ROBOT_Z * this->pnp_cfg.BUFF_ROBOT_Z);

    this->position_in_ptz.at<double>(2, 0) = this->distance;

    // 判断深度信息的有效性，若无效，则退出
    // 修正z轴数据（深度）(模型或补偿系数)TODO
    // cam->ptz坐标系转换
    transformCamera2PTZ(position_in_camera, position_in_ptz);

    const double* _xyz = (const double*)this->position_in_ptz.data;

#ifdef MODEL
    double xyz[3] = {_xyz[0], _xyz[1] - offset_gravity, this->distance};
#endif  // DEBUG
    double xyz[3] = {_xyz[0], _xyz[1] - offset_gravity, _xyz[2]};

    // yaw
    _messenger->getSendInfo().angle_yaw = static_cast<float>(atan2(xyz[0], xyz[2]) * 180 / CV_PI);

    // pitch
    _messenger->getSendInfo().angle_pitch =
        -getPitch(xyz, this->distance / 1000, this->target_h / 1000,
                  _messenger->getReceiveInfo().bullet_volacity) *
        180 / CV_PI;
}

/**
 * @brief 获取补偿 pitch 轴角度
 * @param  xyz              目标位移向量
 * @param  dist             目标深度
 * @param  tvec_y           目标高度
 * @param  _bullet_velocity 子弹速度 mm/s
 * @return float
 */
float RM_SolvePnP::getPitch(double* xyz, float dist, float tvec_y, float _bullet_velocity)
{
    // 申明临时y轴方向长度,子弹实际落点，实际落点与击打点三个变量不断更新（mm）
    float y_temp, y_actual, dy;
    // 重力补偿枪口抬升角度
    float a       = 0.0;
    float GRAVITY = 10000.7f;  //加上空气阻力 徐玺注.
    y_temp        = tvec_y;
    // 迭代求抬升高度
    for (int i = 0; i < 20; i++) {
        // 计算枪口抬升角度
        a = (float)atan2(y_temp, dist);
        // 计算实际落点
        float t;
        t        = dist / _bullet_velocity * cos(a);
        y_actual = _bullet_velocity * sin(a) * t - GRAVITY * t * t / 2;
        dy       = tvec_y - y_actual;
        y_temp   = y_temp + dy;
        // cout<<"i ="<<i<<endl;
        // 当枪口抬升角度与实际落点误差较小时退出
        if (fabsf(dy) < 0.01) {
            break;
        }
    }
    return a;
}

/**
 * @brief 绘制 3d 坐标系
 * @param  input_img        画板
 */
void RM_SolvePnP::drawCoordinate(cv::Mat& input_img)
{
    reference_Obj.clear();
    reference_Obj.emplace_back(cv::Point3f(0.0, 0.0, 0.0));
    reference_Obj.emplace_back(cv::Point3f(100, 0.0, 0.0));
    reference_Obj.emplace_back(cv::Point3f(0.0, 100, 0.0));
    reference_Obj.emplace_back(cv::Point3f(0.0, 0.0, 100));

    projectPoints(reference_Obj, position_in_camera_rvec, position_in_camera, camera_matrix,
                  distortion_coeff, reference_Img);

    cv::Point2f offset_draw = cv::Point2f(0, 112);
    line(input_img, reference_Img[0] - offset_draw, reference_Img[1] - offset_draw,
         cv::Scalar(0, 0, 255), 2);
    line(input_img, reference_Img[0] - offset_draw, reference_Img[2] - offset_draw,
         cv::Scalar(0, 255, 0), 2);
    line(input_img, reference_Img[0] - offset_draw, reference_Img[3] - offset_draw,
         cv::Scalar(255, 0, 0), 2);

    reference_Img.clear();
    reference_Obj.clear();

    std::vector<cv::Point2f>(reference_Img).swap(reference_Img);
    std::vector<cv::Point3f>(reference_Obj).swap(reference_Obj);

    if (this->pnp_cfg.IS_DRAW_COORDINATE == 1) {
        imshow("drawCoordinate", input_img);
    }
}

void RM_SolvePnP::reset()
{
    // Mat
    // vector
    // value
}

void RM_SolvePnP::offsetManually(RM_Messenger* _messenger)
{
    cout << "angle_pitch = " << _messenger->getSendInfo().angle_pitch << endl;

    // // 滑动条输入
    // cv::namedWindow("Armor_Pitch_Trackbar");

    // cv::createTrackbar("offset_yaw_symbol 1:正 0:负 =", "Armor_Pitch_Trackbar",
    //                    &this->offset_yaw_symbol, 1, nullptr);
    // cv::createTrackbar("offset_armor_yaw / 10=", "Armor_Pitch_Trackbar", &this->offset_armor_yaw,
    //                    100, nullptr);
    // cv::createTrackbar("offset_pitch_symbol 1:正 0:负 =", "Armor_Pitch_Trackbar",
    //                    &this->offset_pitch_symbol, 1, nullptr);
    // cv::createTrackbar("offset_armor_pitch /10 =", "Armor_Pitch_Trackbar",
    // &this->offset_armor_yaw,
    //                    100, nullptr);

    // // int转换回float
    // this->pnp_cfg.OFFSET_ARMOR_PITCH = this->offset_armor_pitch * 0.1;
    // this->pnp_cfg.OFFSET_ARMOR_YAW   = this->offset_armor_yaw * 0.1;

    /* 正负分开传值 */
    // pitch
    // if (this->offset_pitch_symbol == 1) {
    // 正：头往下走
    _messenger->getSendInfo().angle_pitch += this->pnp_cfg.OFFSET_ARMOR_PITCH;
    // }
    // else {
    // 负：头往上抬
    //     _messenger->getSendInfo().angle_pitch -= this->pnp_cfg.OFFSET_ARMOR_PITCH;
    // }
    // yaw
    // if (this->offset_yaw_symbol == 1) {
    _messenger->getSendInfo().angle_yaw += this->pnp_cfg.OFFSET_ARMOR_YAW;
    // }
    // else {
    //     _messenger->getSendInfo().angle_yaw -= this->pnp_cfg.OFFSET_ARMOR_YAW;
    // }

    // 显示修改
    cout << "angle_pitch = " << _messenger->getSendInfo().angle_pitch << endl;
    // cv::imshow("Armor_Pitch_Trackbar",offset_trackbar_img);
}

RM_SolvePnP::~RM_SolvePnP() {}