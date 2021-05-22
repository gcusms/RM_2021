#pragma once

#include "Basis/RM_Control/Debug_Controller.h"
#include "Tools/RM_Messenger/RM_Messenger.h"
#include <errno.h>  //ERROR数字定义
#include <fcntl.h>  //文件控制定义
#include <iostream>
#include <string.h>
#include <sys/select.h>
#include <termios.h>  //POSIX终端控制定义
#include <unistd.h>   //UNIX标准定义
using namespace std;

typedef struct Serial_Config
{
    int SHOW_SERIAL_INFORMATION;
    Serial_Config()
    {
        SHOW_SERIAL_INFORMATION = 1;
    }
} Serial_Cfg;

class SerialPort {
  private:
    // CRC校验位的开关 0：关 1：开
    bool isopenCRC;  // TODO: 还未使用

    //串口标志量
    static int           fd;
    static unsigned char g_write_buf[WRITE_BUFF_LENGTH];
    static unsigned char g_CRC_buf[CRC_BUFF_LENGTH];
    static unsigned char g_rec_buf[REC_BUFF_LENGTH];
    static unsigned char g_rec_buf_temp[REC_BUFF_LENGTH];

    // 高低八位的数据还原
    // 发送
    static int16_t _yaw_reduction;  // TODO:是否可以改为 uint16_t
    static int16_t _pitch_reduction;
    static int16_t _depth_reduction;

    // 接收
    static int16_t _angle_reduction;  // TODO:是否可以改为 uint16_t
    static int16_t _acceleration_reduction;

    //交接返回值
    static unsigned char exchangebyte;
    static int16_t       exchangebit;

    /** ---------- 函数声明 ---------- **/
    // CRC校验函数
    static uint8_t Checksum_CRC8(unsigned char* buf, uint16_t len);
    // 数据分解函数
    static void getDataForCRC(const int&     data_type,
                              const int&     is_shooting,
                              const int&     _yaw,
                              const int16_t& yaw,
                              const int&     _pitch,
                              const int16_t& pitch,
                              const int16_t& depth);
    static void getDataForSend(const int&     data_type,
                               const int&     is_shooting,
                               const int&     _yaw,
                               const int16_t& yaw,
                               const int&     _pitch,
                               const int16_t& pitch,
                               const int16_t& depth,
                               const uint8_t& CRC);

  public:
    /** ---------- 函数声明 ---------- **/
    SerialPort(const Serial_Cfg& _serial_config);
    ~SerialPort();

    // 拆开成位 bit
    static inline unsigned char returnHighBit(const int16_t& Byte)
    {
        exchangebyte = (Byte >> 8) & 0xff;
        return exchangebyte;
    }
    static inline unsigned char returnLowBit(const int16_t& Byte)
    {
        exchangebyte = Byte & 0xff;
        return exchangebyte;
    }

    // 合成为字节 Byte
    static inline int16_t mergeIntoBytes(const unsigned char& highbit, const unsigned char& lowbit)
    {
        exchangebit = (highbit << 8) | (lowbit);
        return exchangebit;
    }

    //自定义串口发送
    static void RMserialWrite(RM_Messenger* _messenger);

    //接收并处理串口数据
    static void RMreceiveData(unsigned char* output);

  private:
    static Serial_Cfg serial_config;
};