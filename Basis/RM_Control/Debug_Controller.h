#pragma once
#include <string>

// #define DEBUG
#define DEBUG_MESSENGER

// 记得修改地址
#define DATAEXCHANGE_FILENAME "/home/gcurobot_nuc05/RM_2021/RM_code/RM_2020_vision_code/Tools/RM_DataExchange/param_config.xml"
// #define DATAEXCHANGE_FILENAME "/home/nuc/workspace/RM_code/RM_2020_vision_code/Tools/RM_DataExchange/param_config.xml" 
// #define DATAEXCHANGE_FILENAME "/home/gcurobot/workspace/RM_code/RM_2020_vision_code/Tools/RM_DataExchange/param_config.xml" //nuc02 infantry
// #define DATAEXCHANGE_FILENAME "/home/gcu/RM_update/RM_code/RM_2020_vision_code/Tools/RM_DataExchange/param_config.xml"// sentry
// #define DATAEXCHANGE_FILENAME "/home/gcu/workspace/RM/WCJ/RM_code/RM_2020_vision_code/Tools/RM_DataExchange/param_config.xml" 
/*-------------------- 串口设置部分 -----------------------*/

enum BuffLength {
    // 接收
    REC_INFO_LENGTH = 8,   // 接受后解码得到的数组长度 TODO: 还未使用
    REC_BUFF_LENGTH = 16,  // 接收的 BUFF 的长度 11+4+1
    // 发送
    CRC_BUFF_LENGTH   = 11,  // 写入 CRC 校验的 BUFF 长度
    WRITE_BUFF_LENGTH = 13,  // 写入串口的 BUFF 长度
    /**
     * @brief 串口各个数组的长度设置
     */
};

/*-------------------- 数据交换中转结构体 -----------------------*/
// debug开关
typedef struct Setting_Config
{
    //注释见param_config.xml文件
    short int IS_OPEN;

    // fps
    int COUT_FPS;
    // camera_config
    int         CAMERA_OR_VEDIO;
    int         USB_CAPTURE;
    std::string AVI_CAPTURE;

    // config
    int MY_MODE;
    int MY_COLOR;
    int MY_ROBOT_ID;
    int BULLET_VOLACITY;
    int IS_SERIAL_OPEN;
    int SHOW_OUTPUT_IMG;
    int IS_PARAM_ADJUSTMENT;
    int ANALYZE_EACH_FRAME;
    int SHOW_DEBUG_INFORMATION;

    Setting_Config()
    {
        IS_OPEN = 0;
        // camera_config
        CAMERA_OR_VEDIO = 0;
        // config
        COUT_FPS                = 1;
        MY_MODE                 = 0;
        MY_COLOR                = 0;
        MY_ROBOT_ID             = 0;
        BULLET_VOLACITY         = 15;
        IS_SERIAL_OPEN          = 1;
        SHOW_DEBUG_INFORMATION  = 0;
        SHOW_OUTPUT_IMG         = 0;
        IS_PARAM_ADJUSTMENT     = 0;
        ANALYZE_EACH_FRAME      = 0;
    }
} Setting_Cfg;
/*-------------------- 数据交换中转结构体 -----------------------*/

/*-------------------- 信息储存结构体以及枚举 -----------------------*/
enum color {
    ALL_COLOR,
    RED,
    BLUE,
    /**
        @brief: 描述己方颜色信息
        @param: ALL_COLOR     无颜色信息，两种颜色都识别
        @param: RED           己方为红色
        @param: BLUE          己方为蓝色
      */
};

enum run_mode {
    DEFAULT_MODE,
    SUP_SHOOT,
    ENERGY_AGENCY,
    SENTRY_MODE,
    BASE_MODE,
    /**
        @brief: 描述运行模式信息
        @param: SUP_SHOOT         自瞄模式
        @param: ENERGY_AGENCY     神符模式
        @param: SENTRY_MODE       SENTRY模式
        @param: BASE_MODE         BASE模式
      */
};

enum armor_size {
    BIG_ARMOR,    // TODO可考虑省略
    SMALL_ARMOR,  // TODO可考虑省略
    BUFF_ARMOR,
    /**
        @brief: 描述装甲板尺寸信息
        @param: BIG_ARMOR             大装甲
        @param: BIG_ARMOR_SIZE_W      大装甲实际宽度
        @param: BIG_ARMOR_SIZE_H      大装甲实际高度
        @param: SMALL_ARMOR_SIZE_W    小装甲实际宽度
        @param: SMALL_ARMOR_SIZE_H    小装甲实际高度
        @param: LIGHT_SIZE_W          灯条实际宽度
        @param: LIGHT_SIZE_H          灯条实际高度
      */
    BIG_ARMOR_SIZE_W   = 250,  // 225 250
    BIG_ARMOR_SIZE_H   = 65,   // 55
    SMALL_ARMOR_SIZE_W = 140,  // 125 140
    SMALL_ARMOR_SIZE_H = 60,   // 55 60
    LIGHT_SIZE_W       = 14,   // 10 15 14
    LIGHT_SIZE_H       = 57,   // 55 58 57
};

enum communicationl_plan {
    COORDINATE,
    ANGLE,
    /**
        @brief: 描述串口通讯方式信息
        @param: COORDINATE    二维坐标
        @param: ANGLE         角度
      */
};

enum Robot_ID {
    HERO = 1,
    ENGINEERING,
    INFANTRY,
    UAV = 6,
    SENTRY,
    /**
        @brief: 描述当前机器人ID信息
        @param: HERO          英雄
        @param: ENGINEERING   工程
        @param: INFANTRY      步兵
        @param: UAV           无人机
        @param: SENTRY        哨兵
      */
};
/*-------------------- 信息储存结构体以及枚举 -----------------------*/