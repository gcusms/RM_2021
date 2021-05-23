#pragma once
#ifndef RM_KALMANFILTER_H
#define RM_KALMANFILTER_H

#include "Basis/RM_Control/Debug_Controller.h"
#include "Tools/RM_Messenger/RM_Messenger.h"
#include <cmath>
#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


class RM_kalmanfilter
{
public:
    RM_kalmanfilter();
    ~RM_kalmanfilter();
    Point2f predict_point(Point _p, float _t);
    void reset();

    float anti_range = 1.5;

private:
    cv::KalmanFilter KF_;
    Mat measurement_matrix;
    int CAMERA_RESOLUTION_COLS = 1280;
    int CAMERA_RESOLUTION_ROLS = 800;
    double t = 0.005;
    float x = CAMERA_RESOLUTION_COLS*0.5;
    float y = CAMERA_RESOLUTION_ROLS*0.5;
};

#endif // RM_KALMANFILTER_H
