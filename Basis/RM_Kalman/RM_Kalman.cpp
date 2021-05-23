#include "Basis/RM_Kalman/RM_Kalman.h"

RM_kalmanfilter::RM_kalmanfilter()
    :KF_(4,2)
{
    measurement_matrix = Mat::zeros(2,1,CV_32F);
    t = 0.005;
    KF_.transitionMatrix = (Mat_<float>(4, 4) << 1, 0, t, 0,
                                                 0, 1, 0, t,
                                                 0, 0, 1, 0,
                                                 0, 0, 0, 1);

    setIdentity(KF_.measurementMatrix, Scalar::all(1));
    setIdentity(KF_.processNoiseCov, Scalar::all(1e-3));//
    setIdentity(KF_.measurementNoiseCov, Scalar::all(1e-2));//测量协方差矩阵R，数值越大回归越慢
    setIdentity(KF_.errorCovPost, Scalar::all(1));

    KF_.statePost = (Mat_<float>(4,1)<< x, y, 0, 0);//初始值
}

RM_kalmanfilter::~RM_kalmanfilter(){

}

Point2f RM_kalmanfilter::predict_point(Point _p, float _t)
{
    t = _t;
    Mat prediction = KF_.predict();
    Point2f predict_pt = Point2f(prediction.at<float>(0),prediction.at<float>(1));

    measurement_matrix.at<float>(0,0) = _p.x;
    measurement_matrix.at<float>(1,0) = _p.y;

    KF_.correct(measurement_matrix);

    Point2f anti_kalmanPoint;
    if((_p.x + anti_range*(_p.x - predict_pt.x))<=CAMERA_RESOLUTION_COLS
        || (_p.x + anti_range*(_p.x - predict_pt.x))>=0)
    {
        if(abs(_p.x - predict_pt.x) > 10)
            anti_kalmanPoint.x = _p.x + anti_range*(_p.x - predict_pt.x);
        else
            anti_kalmanPoint.x = _p.x;
    }
    else
    {
        anti_kalmanPoint.x = _p.x;
    }
    anti_kalmanPoint.y = _p.y;

    return anti_kalmanPoint;
}

void RM_kalmanfilter::reset()
{
    measurement_matrix = Mat::zeros(2,1,CV_32F);
    t = 0.005; // FPS 200
}
