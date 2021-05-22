#include "Basis/RM_Link/RM_Link.h"

/**
 * @brief Construct a new rm link::rm link object
 * 选择输入源并初始化连接类
 * @note 先初始化文件交互类和工业相机类
 */
RM_Link::RM_Link()
    : data_exchange(make_unique<RM_DataExchange>(DATAEXCHANGE_FILENAME)),
      industrialcapture(make_unique<RM_VideoCapture>(data_exchange->camera_cfg))
{
    // 选择初始化videocapture
    if (this->data_exchange->camera_cfg.ISOPEN_INDUSTRY_CAPTURE != 0) {
        if (this->data_exchange->setting_cfg.CAMERA_OR_VEDIO == 0) {
            // 初始化usb摄像头读取方式
            capture = make_unique<VideoCapture>(data_exchange->setting_cfg.USB_CAPTURE);
        }
        else {
            // 初始化视频读取方式
            capture = make_unique<VideoCapture>(data_exchange->setting_cfg.AVI_CAPTURE);
        }
    }

    // 初始化角度解算器
    this->solver = make_unique<RM_SolvePnP>(data_exchange->pnp_cfg);

    // 初始化串口数据交互类
    this->messenger = make_unique<RM_Messenger>(data_exchange->messneger_cfg);

    // 初始化串口
    if (data_exchange->setting_cfg.IS_SERIAL_OPEN == 1) {
        this->serialport = make_unique<SerialPort>(data_exchange->serial_cfg);
    }
    else {
        this->serialport->~SerialPort();
    }

    // 卡尔曼预测 TODO

    // 初始化帧率计算模块
    this->fps = make_unique<RM_FPS>();

    // 初始化自瞄模块
    this->armor = make_unique<RM_Armor>(data_exchange->armor_cfg, data_exchange->roi_cfg);
    // 初始化能量机关模块 TODO
    // 初始化录制模块 TODO

    // 初始化图像
    frame   = make_unique<Mat>();
    src_img = make_unique<Mat>();
}

/**
 * @brief Destroy the rm link::rm link object
 *
 */
RM_Link::~RM_Link()
{
    // TODO:清楚内存或其他内容
}

/**
 * @brief 总运行文件
 *
 */
void RM_Link::run()
{
    // 初始化 pnp 参数
    this->solver->setRelationPoseCameraPTZ();
    this->solver->set3dPoint();

    int num =0;//计数
    int temp = 0;//计数
    int fire = 0;
    float yaw = 0;
    float lost_yaw  = 0;
    while (true) {
        num++;//
        cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << endl;

        // 记录时间点
        if (this->data_exchange->setting_cfg.COUT_FPS == 1) {
            this->fps->getTick();
        }

        if (industrialcapture->isindustryimgInput()) {
            // 读取图像
            *frame = cvarrToMat(industrialcapture->iplImage, true);
        }
        else {
            *capture >> *frame;
        }

        // 判断异常
        if (frame->empty()) {
            cerr << "❌❌❌❌❌图像为空❌❌❌❌❌" << endl;
            break;
        }

        // 另存图像
        this->frame->copyTo(*src_img);

        //读取串口数据
        if (this->data_exchange->setting_cfg.IS_SERIAL_OPEN == 1) {
            SerialPort::RMreceiveData(this->messenger->getReceiveArray());
            this->messenger->updateReceiveInformation();
        }

        // TODO：手动选择模式（读取按键或文件内交互）
        if (this->data_exchange->setting_cfg.IS_PARAM_ADJUSTMENT != 0) {
            // 颜色
            if (this->data_exchange->setting_cfg.MY_COLOR != 0) {
                this->messenger->getReceiveInfo().my_color =
                    this->data_exchange->setting_cfg.MY_COLOR;
            }
            // 模式
            if (this->data_exchange->setting_cfg.MY_MODE != 0) {
                this->messenger->getReceiveInfo().now_run_mode =
                    this->data_exchange->setting_cfg.MY_MODE;
            }
            // id
            if (this->data_exchange->setting_cfg.MY_ROBOT_ID != 0) {
                this->messenger->getReceiveInfo().my_robot_id =
                    this->data_exchange->setting_cfg.MY_ROBOT_ID;
            }
            // 射速等级
            if (this->data_exchange->setting_cfg.BULLET_VOLACITY != 0) {
                this->messenger->getReceiveInfo().bullet_volacity =
                    this->data_exchange->setting_cfg.BULLET_VOLACITY;
            }
            // 显示手动修改后的数据
            this->messenger->displayReceiveInformation();
        }

        // 模式选择
        // this->messenger->setSendLostInfo();

        switch (this->messenger->getReceiveInfo().now_run_mode) {
            case SUP_SHOOT:
                // 模式切换后的数据重置 TODO

                // 地面单位模式的自瞄
                // 成功进行串口数据的准备，否则则按需发送数据（一定时间内用预测值，一定时间后发送0）
                if (this->armor->identificationTask(*frame, this->messenger.get())) {
                     if(num %4 == 0)
                    {
                        if(temp %2 == 0)
                        {
                            lost_yaw = this->messenger->getSendInfo().angle_yaw;
                        }
                        else{

                            yaw = this->messenger->getSendInfo().angle_yaw - lost_yaw;
                            if(yaw < 0.5)
                            {
                                yaw =0;
                            }
                        }
                        temp++;
                    }
                    //         cout<<"yaw = "<<yaw<<endl;
                    //         cout<<"yaw = "<<yaw<<endl;
                    //         cout<<"yaw = "<<yaw<<endl;
                    //         cout<<"yaw = "<<yaw<<endl;
                    // 卡尔曼预测 TODO

                    // 角度结算

                    this->solver->runSolvePnP(this->armor->returnTarget2DPoint(),
                                              this->armor->returnFinalArmor().returnArmorType(),
                                              *this->src_img);

                    // 获取角度
                    this->solver->getAngle(this->messenger.get(), yaw);

                    // 打印pnp的相关数据，目前只有深度信息
                    if (this->data_exchange->pnp_cfg.SHOW_PNP_INFORMATION == 1) {
                        Point put_distance =
                            Point(this->armor->returnFinalArmor().returnRect().center.x,
                                  this->armor->returnFinalArmor().returnRect().center.y - 10);
                        putText(*src_img, to_string(this->messenger->getSendInfo().depth),
                                put_distance, FONT_HERSHEY_PLAIN, 1, Scalar(127, 255, 0), 1, 8,
                                false);
                    }
                        if(this->messenger->getSendInfo().angle_yaw < 0.2 && this->messenger->getSendInfo().angle_pitch < 0.2 && this->messenger->getSendInfo().depth < 4000)
                        {
                            this->messenger->getSendInfo().command = 1;
                            cout<<"开炮开炮开炮开炮开炮开炮开炮开炮"<<endl;
                            fire++;
                            if(fire > 100)
                            {
                                this->messenger->getSendInfo().command = 2;
                            }
                        }
                        else{
                            fire =0; 
                        }

                    // 自动控制开火 TODO
                }
                // 选择性跳过
                break;
            case ENERGY_AGENCY:
                // 模式切换后的数据重置

                // 能量机关

                // 选择性跳过
                // 卡尔曼预测
                // 角度结算
                // 获取角度
                // 自动控制开火 TODO
                break;
            case SENTRY_MODE:
                // 模式切换后的数据重置

                // 哨兵模式的自瞄

                // 选择性跳过
                // 卡尔曼预测
                // 角度结算
                // 获取角度
                // 自动控制开火 TODO
                break;
            case BASE_MODE:
                // 模式切换后的数据重置

                // 前哨站模式的自瞄

                // 选择性跳过
                // 卡尔曼预测
                // 角度结算
                // 获取角度
                // 自动控制开火 TODO
                break;
            default:
                // 模式切换后的数据重置

                // 自瞄

                // 选择性跳过
                // 卡尔曼预测
                // 角度结算
                // 获取角度
                // 自动控制开火 TODO

                break;
        }

        // 自动控制开火 TODO

        //发送串口数据
        if (this->data_exchange->setting_cfg.IS_SERIAL_OPEN == 1) {
            SerialPort::RMserialWrite(this->messenger.get());
        }

        // 显示原图 没有显示屏的时候记得注释掉
        if (this->data_exchange->setting_cfg.SHOW_OUTPUT_IMG == 1 &&
            this->data_exchange->setting_cfg.IS_PARAM_ADJUSTMENT == 1) {
            // line(*src_img, Point(640, 0), Point(640, 800), Scalar(255, 255, 255), 2, 8, 0);
            // line(*src_img, Point(0, 400), Point(1280, 400), Scalar(255, 255, 255), 2, 8, 0);
            imshow("frame", *frame);
            imshow("src_img", *src_img);
        }

        // 释放相机内容（必须！）
        industrialcapture->cameraReleasebuff();

// #ifndef RELEASE
        // 按 q 退出程序
        if (waitKey(1) == 'q') {
            destroyAllWindows();
            break;
        }
// #else
        // usleep(1);
// #endif  // !RELEASE

        cout << "COUT_FPS = " << this->data_exchange->setting_cfg.COUT_FPS << endl;
        // 计算运行时间
        if (this->data_exchange->setting_cfg.COUT_FPS == 1) {
            this->fps->calculateFPS();
        }
        cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << endl;
        cout << endl;
    }
}
