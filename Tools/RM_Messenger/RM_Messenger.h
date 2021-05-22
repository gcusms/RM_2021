#pragma once
#include "Basis/RM_Control/Debug_Controller.h"
#include <iostream>
#include <stdint.h>
#include <math.h>

using namespace std;

/**
 * @brief 接收信息
 */
typedef struct Receive_Information
{
    int   my_color;         
    int   now_run_mode;     
    int   my_robot_id;      
    float acceleration;     

    union IMU_Yaw_Angle_Information
    {
        float   IMU_Yaw_angle;
        uint8_t arr_Yaw_angle[4];
    } IMU_Yaw_Angle_Info;

    union IMU_Pitch_Angle_Information
    {
        float   IMU_Pitch_angle;
        uint8_t arr_Pitch_angle[4];
    } IMU_Pitch_Angle_Info;

    int bullet_volacity;   

    Receive_Information()
    {
        my_color                             = ALL_COLOR;
        now_run_mode                         = SUP_SHOOT;
        my_robot_id                          = INFANTRY;
        IMU_Yaw_Angle_Info.IMU_Yaw_angle     = 0.f;
        IMU_Pitch_Angle_Info.IMU_Pitch_angle = 0.f;
        acceleration                         = 0.f;
        bullet_volacity                      = 15;
    }
    /**
     * @brief   串口信息接收结构体
     * @param:  my_color              描述己方颜色信息
     * @param:  now_run_mode          描述运行模式信息
     * @param:  my_robot_id           描述当前机器人ID信息
     * @param:  IMU_Yaw_Angle_Info    描述陀螺仪的yaw轴角度（有符号）
     * @param:  IMU_Pitch_Angle_Info  描述陀螺仪的pitch轴角度（有符号）
     * @param:  acceleration          描述陀螺仪角加速度 .00
     * @param:  bullet_volacity       描述当前机器人子弹速度
     */
} Receive_Info;

/**
 * @brief 发送信息
 */
typedef struct Send_Information
{
    // 最终发送
    float angle_yaw;
    float angle_pitch;
    int   symbol_yaw;
    int   symbol_pitch;
    int   command;
    int   is_found_target;
    float depth;
    /**
     * @brief   串口信息发送结构体
     * @param:  angle_yaw         yaw 轴角度
     * @param:  angle_pitch       pitch 轴角度
     * @param:  symbol_yaw        yaw轴符号（右正左负）
     * @param:  symbol_pitch      pitch轴符号（下正上负）
     * @param:  command           机器人的开火指令：0->不开火 1->单发开火一枪 2->跟随 3->不跟随
     * @param:  is_found_target   是否发现目标：1->发现 0->未发现
     * @param:  depth             深度信息
     */

} Send_Info;

// 信使类参数配置表
typedef struct Messenger_Config
{
    int SHOW_SERIAL_INFORMATION;
    Messenger_Config()
    {
        SHOW_SERIAL_INFORMATION = 0;
    }
} Messenger_Cfg;

// 数据传输
class RM_Messenger {
  public:
    // 初始化数据
    RM_Messenger(const Messenger_Config& _messenger_config);

    ~RM_Messenger(){};

  public:
    // 接受
    void updateReceiveInformation();
    // 显示接收数据
    void displayReceiveInformation();

    // 返回串口接收数组
    unsigned char* getReceiveArray();
    // 获取接收数据
    Receive_Info& getReceiveInfo();

  public:
    // 发送
    // 获取发送数据
    Send_Info& getSendInfo();
    void       setSendLostInfo();
    void       setSymbol();

  private:
    Receive_Info receive_info;        // 当前接收的信息
    Receive_Info last_receive_info;   // 上一帧接收的信息
    Send_Info    send_info;           // 当前需要发送的消息

  private:
    Messenger_Cfg messneger_cfg;

    unsigned char receive_arr[REC_BUFF_LENGTH];  // 待解码的串口字符串长度
    int           transform_arr[4];              // 标志位数组

    // 判断接收数据是否为空（0）
    bool isEmpty();
};