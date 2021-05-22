#pragma once
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

/**
 * @brief 帧率类
 *
 */
class RM_FPS {
  public:
    RM_FPS();
    ~RM_FPS();
    void calculateFPS();
    void getTick();

  private:
    void displayFPS() const;

    int    cnt;      //  计算次数
    double time1;    //  记录第一次时间点
    double time2;    //  记录第二次时间点
    double time;     //  记录时间段
    double fps;      //  帧率
    double max;      //  最大帧率
    double min;      //  最小帧率
    double average;  //  平均帧率（去除前10帧）
    double total;    //  总帧率
};