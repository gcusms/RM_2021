#include "Tools/RM_Messenger/RM_Messenger.h"

/**
 * @brief Construct a new rm messenger::rm messenger object
 * @param  _messenger_configMy Param doc
 */
RM_Messenger::RM_Messenger(const Messenger_Config& _messenger_config)
    : messneger_cfg(_messenger_config)
{
}

/**
 * @brief 更新串口接收数据
 */
void RM_Messenger::updateReceiveInformation()
{
    if (this->isEmpty()) {
        this->receive_info = this->last_receive_info;
        if (this->messneger_cfg.SHOW_SERIAL_INFORMATION == 1) {
            this->displayReceiveInformation();
        }
        return;
    }

    // 更新并保存为上一帧的数据
    last_receive_info = receive_info;

    //转换类型为 int
    for (size_t i = 0; i < sizeof(transform_arr) / sizeof(transform_arr[0]); ++i) {
        this->transform_arr[i] =
            this->receive_arr[i + 1] - '0';  //需要 - '0',因为char转int会剩下ascII码的值
    }

    /* 1 更新颜色信息　update color */
    switch (transform_arr[0]) {
        case RED:
            this->receive_info.my_color = RED;
            break;
        case BLUE:
            this->receive_info.my_color = BLUE;
            break;
        default:
            this->receive_info.my_color = ALL_COLOR;
            break;
    }

    /* 2 更新模式信息 update mode */
    switch (transform_arr[1]) {
        case SUP_SHOOT:
            this->receive_info.now_run_mode = SUP_SHOOT;
            break;
        case ENERGY_AGENCY:
            this->receive_info.now_run_mode = ENERGY_AGENCY;
            break;
        case SENTRY_MODE:
            this->receive_info.now_run_mode = SENTRY_MODE;
            break;
        case BASE_MODE:
            this->receive_info.now_run_mode = BASE_MODE;
            break;
        default:
            this->receive_info.now_run_mode = SUP_SHOOT;
            break;
    }

    /* 3 更新当前机器人ID update Robot ID */
    switch (transform_arr[2]) {
        case HERO:
            this->receive_info.my_robot_id = HERO;
            break;
        case ENGINEERING:
            this->receive_info.my_robot_id = ENGINEERING;
            break;
        case INFANTRY:
            this->receive_info.my_robot_id = INFANTRY;
            break;
        case UAV:
            this->receive_info.my_robot_id = UAV;
            break;
        case SENTRY:
            this->receive_info.my_robot_id = SENTRY;
            break;
        default:
            this->receive_info.my_robot_id = INFANTRY;
            break;
    }

    /* 4 5 6 7 更新陀螺仪的yaw角度值 */
    for (size_t i = 0; i < sizeof(this->receive_info.IMU_Yaw_Angle_Info.arr_Yaw_angle); ++i) {
        // +4是接收串口数组中陀螺仪yaw角度值的起始位置
        this->receive_info.IMU_Yaw_Angle_Info.arr_Yaw_angle[i] = this->receive_arr[i + 4];
    }

    /*8 9 10 11 更新陀螺仪的pitch角度值 */
    for (size_t i = 0; i < sizeof(this->receive_info.IMU_Pitch_Angle_Info.arr_Pitch_angle); ++i) {
        // +8是接收串口数组中陀螺仪pitch角度值的起始位置
        this->receive_info.IMU_Pitch_Angle_Info.arr_Pitch_angle[i] = this->receive_arr[i + 8];
    }

    /* 12 13 更新陀螺仪的角加速度值 */
    this->receive_info.acceleration =
        static_cast<float>((this->receive_arr[12] << 8) | (this->receive_arr[13])) * 0.01f;

    /* 14 更新子弹速度 */
    // this->transform_arr[3] = this->receive_arr[14] - '0';
    // switch (this->transform_arr[3]) {
    //     case 1:
    this->receive_info.bullet_volacity = this->receive_arr[14];
    //         break;
    //     case 2:
    //         this->receive_info.bullet_volacity = 18;
    //         break;
    //     case 3:
    //         this->receive_info.bullet_volacity = 30;
    //         break;
    //     case 4:
    //         this->receive_info.bullet_volacity = 10;
    //         break;
    //     case 5:
    //         this->receive_info.bullet_volacity = 16;
    //         break;
    //     default:
    //         this->receive_info.bullet_volacity = 18;
    //         break;
    // }

    if (this->messneger_cfg.SHOW_SERIAL_INFORMATION == 1) {
        this->displayReceiveInformation();
    }
}

/**
 * @brief 打印当前接收的信息
 */
void RM_Messenger::displayReceiveInformation()
{
    cout << "color:" << this->receive_info.my_color << " model:" << this->receive_info.now_run_mode
         << " ID:" << this->receive_info.my_robot_id
         << " yaw:" << this->receive_info.IMU_Yaw_Angle_Info.IMU_Yaw_angle
         << " pitch:" << this->receive_info.IMU_Pitch_Angle_Info.IMU_Pitch_angle
         << " acceleration:" << this->receive_info.acceleration
         << " volacity:" << this->receive_info.bullet_volacity << endl;

    // cout << "⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️ ⚔️"<<
    // endl;
}

/**
 * @brief 返回串口接收数组
 * @return unsigned*
 */
unsigned char* RM_Messenger::getReceiveArray()
{
    return this->receive_arr;
}

/**
 * @brief 返回接收信息
 * @return Receive_Info&
 */
Receive_Info& RM_Messenger::getReceiveInfo()
{
    return this->receive_info;
}

/**
 * @brief 返回发送信息
 * @return Send_Info&
 */
Send_Info& RM_Messenger::getSendInfo()
{
    return this->send_info;
}

/**
 * @brief 判断接收数据是否为空
 * @return true     为空（0）
 * @return false    不为空
 */
bool RM_Messenger::isEmpty()
{
    if (receive_arr[0] != '0' || receive_arr[REC_BUFF_LENGTH - 1] != '0') {
        return false;
    }
    else {
        return true;
    }
}

/**
 * @brief 发送丢失时的数据
 */
void RM_Messenger::setSendLostInfo()
{
    this->send_info.angle_pitch     = 0;
    this->send_info.angle_yaw       = 0;
    this->send_info.command         = 0;
    this->send_info.depth           = 0;
    this->send_info.is_found_target = 0;
    this->send_info.symbol_pitch    = 0;
    this->send_info.symbol_yaw      = 0;
}

/**
 * @brief 设置符号
 */
void RM_Messenger::setSymbol()
{
    if (this->send_info.angle_pitch >= 0) {
        this->send_info.symbol_pitch = 1;
    }
    else {
        this->send_info.symbol_pitch = 0;
    }

    if (this->send_info.angle_yaw >= 0) {
        this->send_info.symbol_yaw = 1;
    }
    else {
        this->send_info.symbol_yaw = 0;
    }
    this->send_info.angle_pitch = fabs(this->send_info.angle_pitch);
    this->send_info.angle_yaw   = fabs(this->send_info.angle_yaw);
}