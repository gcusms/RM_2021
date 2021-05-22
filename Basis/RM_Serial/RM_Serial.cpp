#include "Basis/RM_Serial/RM_Serial.h"

const unsigned char CRC8Tab[300] = {
    0,   94,  188, 226, 97,  63,  221, 131, 194, 156, 126, 32,  163, 253, 31,  65,  157, 195, 33,
    127, 252, 162, 64,  30,  95,  1,   227, 189, 62,  96,  130, 220, 35,  125, 159, 193, 66,  28,
    254, 160, 225, 191, 93,  3,   128, 222, 60,  98,  190, 224, 2,   92,  223, 129, 99,  61,  124,
    34,  192, 158, 29,  67,  161, 255, 70,  24,  250, 164, 39,  121, 155, 197, 132, 218, 56,  102,
    229, 187, 89,  7,   219, 133, 103, 57,  186, 228, 6,   88,  25,  71,  165, 251, 120, 38,  196,
    154, 101, 59,  217, 135, 4,   90,  184, 230, 167, 249, 27,  69,  198, 152, 122, 36,  248, 166,
    68,  26,  153, 199, 37,  123, 58,  100, 134, 216, 91,  5,   231, 185, 140, 210, 48,  110, 237,
    179, 81,  15,  78,  16,  242, 172, 47,  113, 147, 205, 17,  79,  173, 243, 112, 46,  204, 146,
    211, 141, 111, 49,  178, 236, 14,  80,  175, 241, 19,  77,  206, 144, 114, 44,  109, 51,  209,
    143, 12,  82,  176, 238, 50,  108, 142, 208, 83,  13,  239, 177, 240, 174, 76,  18,  145, 207,
    45,  115, 202, 148, 118, 40,  171, 245, 23,  73,  8,   86,  180, 234, 105, 55,  213, 139, 87,
    9,   235, 181, 54,  104, 138, 212, 149, 203, 41,  119, 244, 170, 72,  22,  233, 183, 85,  11,
    136, 214, 52,  106, 43,  117, 151, 201, 74,  20,  246, 168, 116, 42,  200, 150, 21,  75,  169,
    247, 182, 232, 10,  84,  215, 137, 107, 53};

int           SerialPort::fd;
unsigned char SerialPort::g_write_buf[WRITE_BUFF_LENGTH];
unsigned char SerialPort::g_CRC_buf[CRC_BUFF_LENGTH];
unsigned char SerialPort::g_rec_buf[REC_BUFF_LENGTH];
unsigned char SerialPort::g_rec_buf_temp[REC_BUFF_LENGTH];

int16_t SerialPort::_yaw_reduction   = 0x0000;
int16_t SerialPort::_pitch_reduction = 0x0000;
int16_t SerialPort::_depth_reduction = 0x0000;

unsigned char SerialPort::exchangebyte;
int16_t       SerialPort::exchangebit = 0x0000;

unsigned char rec_buf_temp[REC_BUFF_LENGTH * 2];

Serial_Cfg SerialPort::serial_config;

/**
 * @brief Construct a new Serial Port:: Serial Port object
 * ------------------------------------------------------
 * @param:  波特率,默认为115200
 * --------------------------------------------------------
 * @param:  char parity 是否进行奇偶校验,'Y'表示需要奇偶校验,'N'表示不需要奇偶校验
 * -------------------------------------------------------------
 * @param:  int databits 数据位的个数,默认值为8个数据位
 *----------------------------------------------------------
 * @return: bool  初始化是否成功
 * @note:   在使用其他本类提供的函数前,请先调用本函数进行串口的初始化
 *　　　　　   函数提供了一些常用的串口参数设置
 *           本串口类析构时会自动关闭串口,无需额外执行关闭串口
 * @author: Hzkkk
 *          Rcxxx (revised)
 */
SerialPort::SerialPort(const Serial_Cfg& _serial_config)
{
    // 更新串口部分的控制开关
    this->serial_config = _serial_config;

    cout << "The Serial set ......" << endl;
    const char* DeviceName[4] = {"", "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2"};

    /* WARNING :  终端设备默认会设置为控制终端，因此open(O_NOCTTY不作为控制终端)
     * Terminals'll default to be set as Control Terminals
     */
    struct termios newstate;
    /*打开串口*/
    bzero(&newstate, sizeof(newstate));  //清零
    for (size_t i = 0; i < (sizeof(DeviceName) / sizeof(char*)); ++i) {
        fd = open(DeviceName[i], O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY);
        if (fd == -1) {
            printf("Can't Open Serial Port %s\n", DeviceName[i]);
            continue;
        }
        else {
            printf("Open Serial Port %s Successful\n", DeviceName[i]);
            break;
        }
    }

    /*设置发送波特率*/
    // #if SET_BANDRATE == 115200
    // cfsetospeed(&newstate, B115200);
    // cfsetispeed(&newstate, B115200);
    // #elif SET_BANDRATE == 921600
    cfsetospeed(&newstate, B921600);
    cfsetispeed(&newstate, B921600);
    // #endif

    //本地连线, 取消控制功能 | 开始接收
    newstate.c_cflag |= CLOCAL | CREAD;
    //设置字符大小
    newstate.c_cflag &= ~CSIZE;
    //设置停止位1
    newstate.c_cflag &= ~CSTOPB;
    //设置数据位8位
    newstate.c_cflag |= CS8;
    //设置无奇偶校验位，N
    newstate.c_cflag &= ~PARENB;

    /*阻塞模式的设置*/
    newstate.c_cc[VTIME] = 0;
    newstate.c_cc[VMIN]  = 0;

    /*清空当前串口*/
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &newstate);
}

/**
 * @brief Destroy the Serial Port:: Serial Port object
 */
SerialPort::~SerialPort(void)
{
    if (!close(fd))
        printf("Close Serial Port Successful\n");
}

/**
 *  @brief: 串口数据读取函数
 *  @return: string  返回收到的字符串
 *  @note:   逐字节读取并存到字符串
 *           等待0.01s后结束读取,将所得字符串返回
 *  @authors: Rcxxx
 *            Hzkkk
 *            Wcjjj
 */
void SerialPort::RMreceiveData(unsigned char* output)
{
    memset(g_rec_buf, '0', REC_BUFF_LENGTH);       //清空缓存
    read(fd, rec_buf_temp, sizeof(rec_buf_temp));  //读取串口中的数据
    //对读取到的数据进行遍历排查，直到截取 'S' 开头和 'E'结尾的数据段后保存并退出
    for (int i = 0; i < (int)sizeof(rec_buf_temp); ++i) {
        if (rec_buf_temp[i] == 'S' && rec_buf_temp[i + sizeof(g_rec_buf) - 1] == 'E') {
            for (int j = 0; j < ((int)sizeof(g_rec_buf)); ++j) {
                // 如果这里没进入可以检查双方协议的位数是否正确
                g_rec_buf[j] = rec_buf_temp[i + j];
            }
            break;
        }
    }

    /* 接收协议：
        0:      S
        1:      color
        2:      model
        3:      robot_id
        4~7:    yaw_angle(union)
        8~11:   pitch_angle(union)
        12~13:  acceleration 12:High 13:low
        14:     bullet_volacity 1:15 2:18 3:30 4:10 5:16
        15:     E
     */
    tcflush(fd, TCIFLUSH);

    for (size_t i = 0; i < sizeof(g_rec_buf); ++i) {
        output[i] = g_rec_buf[i];
    }

    if (serial_config.SHOW_SERIAL_INFORMATION == 1) {
        cout << "g_rec_buf:";
        for (size_t i = 0; i < sizeof(g_rec_buf); ++i) {
            cout << static_cast<int>(g_rec_buf[i]) << " ";
            // 有问题先查看串口读取的源头是否被污染
        }
        cout << endl;
    }
}

/**
 *@brief: RM串口发送格式化函数
 *
 * @param: yaw 云台偏航
 * @param: pitch 云台俯仰
 * @param: _yaw yaw正负
 * @param: _pitch pitch正负
 * @param: data_type 是否正确识别的标志
 *
 * @authors: Rcxxx
 *           Hzkkk
 *           Wcjjj
 */
void SerialPort::RMserialWrite(RM_Messenger* _messenger)
{
    getDataForCRC(_messenger->getSendInfo().is_found_target, _messenger->getSendInfo().command,
                  _messenger->getSendInfo().symbol_yaw, static_cast<int16_t>(_messenger->getSendInfo().angle_yaw * 100),
                  _messenger->getSendInfo().symbol_pitch,
                  static_cast<int16_t>(_messenger->getSendInfo().angle_pitch * 100), _messenger->getSendInfo().depth);

    uint8_t CRC = Checksum_CRC8(g_CRC_buf, sizeof(g_CRC_buf));
    getDataForSend(_messenger->getSendInfo().is_found_target, _messenger->getSendInfo().command,
                   _messenger->getSendInfo().symbol_yaw, static_cast<int16_t>(_messenger->getSendInfo().angle_yaw  * 100),
                   _messenger->getSendInfo().symbol_pitch,
                   static_cast<int16_t>(_messenger->getSendInfo().angle_pitch * 100), _messenger->getSendInfo().depth,
                   CRC);
    /*
    0：帧头     1：是否正确识别的标志   2：是否射击的信号
    3：yaw正负值    4：yaw低八位数据    5：yaw高八位数据
    6：pitch正负值  7：pitch低八位数据  8：pitch高八位数据
    9：深度低八位   10：深度高八位
    11：CRC
    12：帧尾
    */
    write(fd, g_write_buf, sizeof(g_write_buf));

    if (serial_config.SHOW_SERIAL_INFORMATION == 1) {
        _yaw_reduction =
            mergeIntoBytes(g_write_buf[5], g_write_buf[4]);  // TODO:测试传出的值是否正确

        _pitch_reduction = mergeIntoBytes(g_write_buf[8], g_write_buf[7]);

        _depth_reduction = mergeIntoBytes(g_write_buf[10], g_write_buf[9]);

        cout << "g_write_buf=  " << g_write_buf[0] << "  " << static_cast<int>(g_write_buf[1])
             << "  " << static_cast<int>(g_write_buf[2]) << "  " << static_cast<int>(g_write_buf[3])
             << "  " << float(_yaw_reduction) / 100 << "  " << static_cast<int>(g_write_buf[6])
             << "  " << float(_pitch_reduction) / 100 << "  " << float(_depth_reduction) << "  "
             << static_cast<int>(g_write_buf[11]) << "  " << g_write_buf[12] << endl;

        _yaw_reduction = 0x0000;  // TODO:测试上述转换方法可行之后看看是否可以去除
        _pitch_reduction = 0x0000;
        _depth_reduction = 0x0000;
    }
}

/** CRC8校验函数
 *
 *  @param:  char *buf   需要检验的字符串
 *  @param:  uint16_t len 是否进行奇偶校验,'Y'表示需要奇偶校验,'N'表示不需要奇偶校验
 *
 *  @return: bool  初始化是否成功
 *  @brief:  CRC8校验 ---MAXIM x8+x5+x4+x1  多项式 POLY（Hex）:31(110001)  初始值 INIT（Hex）：00
 *结果异或值 XOROUT（Hex）：
 *  @note:   在使用其他本类提供的函数前,请先调用本函数进行串口的初始化
 *　　　　　   函数提供了一些常用的串口参数设置
 *           本串口类析构时会自动关闭串口,无需额外执行关闭串口
 */
uint8_t SerialPort::Checksum_CRC8(unsigned char* buf, uint16_t len)
{
    uint8_t check = 0;

    while (len--) {
        check = CRC8Tab[check ^ (*buf++)];
    }

    return (check) /* &0x00ff */;  // TODO:测试去除 &0x00ff ，看是否能够正常接收
}

/**
 * @brief 提取数据进入 CRC 校验数组
 *
 * @param data_type
 * @param is_shooting
 * @param _yaw
 * @param yaw
 * @param _pitch
 * @param pitch
 * @param depth
 */
void SerialPort::getDataForCRC(const int&     data_type,
                               const int&     is_shooting,
                               const int&     _yaw,
                               const int16_t& yaw,
                               const int&     _pitch,
                               const int16_t& pitch,
                               const int16_t& depth)
{
    g_CRC_buf[0]  = 0x53;
    g_CRC_buf[1]  = static_cast<unsigned char>(data_type);
    g_CRC_buf[2]  = static_cast<unsigned char>(is_shooting);
    g_CRC_buf[3]  = static_cast<unsigned char>(_yaw);
    g_CRC_buf[4]  = returnLowBit(yaw);
    g_CRC_buf[5]  = returnHighBit(yaw);
    g_CRC_buf[6]  = static_cast<unsigned char>(_pitch);
    g_CRC_buf[7]  = returnLowBit(pitch);
    g_CRC_buf[8]  = returnHighBit(pitch);
    g_CRC_buf[9]  = returnLowBit(depth);
    g_CRC_buf[10] = returnHighBit(depth);

}

/**
 * @brief 提取数据进入发送的数组
 *
 * @param data_type
 * @param is_shooting
 * @param _yaw
 * @param yaw
 * @param _pitch
 * @param pitch
 * @param depth
 * @param CRC
 */
void SerialPort::getDataForSend(const int&     data_type,
                                const int&     is_shooting,
                                const int&     _yaw,
                                const int16_t& yaw,
                                const int&     _pitch,
                                const int16_t& pitch,
                                const int16_t& depth,
                                const uint8_t& CRC)
{
    g_write_buf[0]  = 0x53;
    g_write_buf[1]  = static_cast<unsigned char>(data_type);
    g_write_buf[2]  = static_cast<unsigned char>(is_shooting);
    g_write_buf[3]  = static_cast<unsigned char>(_yaw);
    g_write_buf[4]  = returnLowBit(yaw);
    g_write_buf[5]  = returnHighBit(yaw);
    g_write_buf[6]  = static_cast<unsigned char>(_pitch);
    g_write_buf[7]  = returnLowBit(pitch);
    g_write_buf[8]  = returnHighBit(pitch);
    g_write_buf[9]  = returnLowBit(depth);
    g_write_buf[10] = returnHighBit(depth);
    g_write_buf[11] = CRC & 0xff;
    g_write_buf[12] = 0x45;

}
