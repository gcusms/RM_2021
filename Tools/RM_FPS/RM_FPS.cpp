#include "Tools/RM_FPS/RM_FPS.h"

/**
 * @brief Construct a new rm fps::rm fps object
 *
 */
RM_FPS::RM_FPS()
{
    cnt     = 0;
    time1   = 0.0;
    time2   = 0.0;
    time    = 0.0;
    fps     = 0.0;
    max     = 0.0;
    min     = 99999999.0;
    average = 0.0;
    total   = 0.0;
}

/**
 * @brief Destroy the rm fps::rm fps object
 *
 */
RM_FPS::~RM_FPS() {}

/**
 * @brief 显示帧率
 *
 */
void RM_FPS::displayFPS() const
{
    // std::cout<<" ---------------------------- "<<std::endl;
    std::cout << "time = " << this->time * 1000 << "毫秒" << std::endl;
    std::cout << "FPS = " << this->fps << std::endl;
    std::cout << "min = " << this->min << " max = " << this->max << std::endl;
    std::cout << "average = " << this->average * 1000 << std::endl;
    // std::cout<<" ---------------------------- <<"<<std::endl;
}

/**
 * @brief 获取时间点
 *
 */
void RM_FPS::getTick()
{
    this->time1 = cv::getTickCount();
}

/**
 * @brief 计算帧率
 *
 */
void RM_FPS::calculateFPS()
{
    this->time2 = cv::getTickCount();
    this->time  = (this->time2 - this->time1) / cv::getTickFrequency();
    this->fps   = 1.f / this->time;
    ++this->cnt;
    if (this->cnt > 10) {
        if (this->fps > this->max) {
            this->max = this->fps;
        }
        if (this->fps < this->min) {
            this->min = this->fps;
        }
        this->total += this->time;
        this->average = this->total / ((this->cnt) - 10);
    }
    this->displayFPS();
}
