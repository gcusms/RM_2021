#include "module/top_detect/top.hpp"

//*****拟合圆形*********//
#define TUOLUO_BUFFER 3         //保留陀螺位置数组容量大小
#define TOP_ANGLE_MIN 3         //两次减陀螺角度差的最小值
#define TOP_CONTINUE_NUM_MIN 5  //认为是陀螺的连续变化次数最小值
#define TOP_CONTINUE_NUM_MAX 9  //为了快速陀螺目标,限定的陀螺累计次数最大值
#define TOP_MIN_DISTANCE 1  //记录陀螺位置点间距离最小值
#define LEASTFITTIMES 40    //最小二乘法拟合数组大小

#define SEC_MODE_SAVE_RUN_TIMES 10  //第二种方法下保存分析移动次数

#define SPACE_TIMES 2  //间隔次数,对该值进行取余,2代表每两次检测一次
#define MIN_VARIANCE 200  //最小方差

//第二种利用10个角度方差计算
float angles[SEC_MODE_SAVE_RUN_TIMES];

//陀螺识别更新保留值
double angle_old = 0;       //上一次识别角度
double continueNumber = 0;  //判断目标连续运动次数
//空间点计算函数
cv::Point3f GetSpaceCenter(cv::Point3f a, cv::Point3f b,
                           cv::Point3f c);  //空间三点计算圆心
cv::Point2f GetSpaceCenter(cv::Point2f point_one, cv::Point2f point_two,
                           cv::Point2f point_three);  //平面三点计算圆心
double GetSpaceDistance(cv::Point3f a, cv::Point3f b);  //空间两点计算距离
//最小二乘法计算圆心
bool circleLeastFit(const std::vector<cv::Point2f> &points, double &center_x,
                    double &center_y, double &radius);
void Draw(vector<cv::Point2f> object, cv::Mat src, cv::Scalar a);

//以下为各个记录值
int getPointTimes = 0;   //记录点次数
float oldSpinSpeed = 0;  //记录上次角速度
cv::Point2f biaozhun;
vector<cv::Point2f> circlePoints;
//以下角度带符号
float max_angle = 0;  //运动过程中角度最大值(已弃用)
float min_angle = 0;  //运动过程中角度最小值(已弃用)
int leftTimes = 0;    //一次连续目标中左转次数
int rightTimes = 0;   //一次连续目标中右转次数
TopRunDirection old_direction =
    LEFT;  //用于应对扭腰情况,次数leftTimes应与rightTimes相差不大
/************************/

/********隔几次记录一次变化,减小帧率,扩大间隔,记录间隔次数(弃用)********/
int SpaceTimes = 0;  //间隔次数
//保留数据
cv::Point2f old_center;
TopStatus old_status;
float old_Speed;
bool old_result;
/********************************************************************/

//利用运动判断左右
#define MIN_RUN_DISTANCE 1   //静止
#define MAX_RUN_DISTANCE 10  //切换装甲
struct tuoluo {
  float angle;        //陀螺角度
  float tx;           //陀螺绝对的tx坐标
  int direction = 1;  //陀螺运动方向
};

tuoluo oldGed;  //计算保留值

/**
 * @brief ShootTuoluo::ShootTuoluo          初始化函数
 */
ShootTuoluo::ShootTuoluo() {
  tz_armor_width_ =
      85.0222;  //标准状态下tz_normal距离对应装甲宽度,该值与tz_normal均通过程序调试时测量获得
  tz_normal = 120.725;  //设定的标准距离
}

TopData ShootTuoluo::getTuoluoData(cv::Mat _src_img,
                                   armor::Armor_Data BestArmor) {
  double angle = getAngle(BestArmor);  //计算相对中心偏转角度,存在一定误差
  TopData tuoluoData;
  if (BestArmor.shooting_status == armor::FirstFind) {
    //首次发现目标各项进行初始化
    firstSetTuoluo(angle, BestArmor);
    biaozhun = cv::Point2f(BestArmor.tx, BestArmor.tz);
    return tuoluoData;
  } else if (BestArmor.shooting_status == armor::Shoot) {
    double R;            //运动半径
    cv::Point2f center;  //运动圆心
    if (ContinueSetTuoluo(_src_img, angle, BestArmor, R, center,
                          tuoluoData.status, tuoluoData.runDirection,
                          tuoluoData.spinSpeed)) {
      //确认对方处于陀螺状态

      //大致绘制出陀螺圆心坐标,存在一定的绘制误差
      float armorWidth = tz_armor_width_ * (tz_normal / center.y);
      drawImage(
          _src_img, angle,
          (BestArmor.left_light.center + BestArmor.right_light.center) / 2, R,
          armorWidth);

      //传参返回
      tuoluoData.isTuoluo = true;
      tuoluoData.angle = angle;
      tuoluoData.R = R;
      tuoluoData.center = center;
      //更新击打点

      return tuoluoData;
    } else {
      //缓冲
      return tuoluoData;
    }
  }
  //不是陀螺,直接返回
  return tuoluoData;
}

/**
 * @brief ShootTuoluo::firstSetTuoluo       首次识别目标将值初始化
 * @param angle         所计算角度
 */
void ShootTuoluo::firstSetTuoluo(double angle, armor::Armor_Data armor) {
  //各项清零
  continueNumber = 0;
  circlePoints.clear();
  max_angle = 0;
  min_angle = 0;
  oldGed.angle = angle;
  oldGed.tx = armor.tx;
  oldGed.direction = 1;
  oldSpinSpeed = 0;
  getPointTimes = 0;
  leftTimes = 0;
  rightTimes = 0;
}

/**
 * @brief ShootTuoluo::ContinueSetTuoluo
 当目标被连续追踪时,判断目标是否处于陀螺状态
 * @param Src                      原始图像
 * @param angle                 所计算的装甲相对中心的偏向角
 * @param BestArmor             当前装甲信息
 *
 * @return
 * @param R                         输出参数:陀螺半径
 * @param center               输出参数:陀螺中心
 * @param status                输出参数:当前陀螺运动状态
 * @param direction          输出参数:运动方向
 * @param spinSpeed       输出参数:旋转速度

 */
bool ShootTuoluo::ContinueSetTuoluo(cv::Mat Src, double &angle,
                                    armor::Armor_Data BestArmor, double &R,
                                    cv::Point2f &center, TopStatus &status,
                                    TopRunDirection &direction,
                                    float &spinSpeed) {
  bool isAngleEffective =
      false;  //记录本次目标角度变化是否符合要求,用于记录转动方向
  bool VarianceIsBig = false;  //记录最近10帧方差大小是否符合波动要求
  //记录是否处于陀螺状态
  if (fabs(angle - fabs(oldGed.angle)) >= TOP_ANGLE_MIN) {
    //符合单帧间角度变化的最小阈值
    continueNumber = (continueNumber + 1) < TOP_CONTINUE_NUM_MAX
                         ? (continueNumber + 1)
                         : TOP_CONTINUE_NUM_MAX;
    isAngleEffective = true;
  } else {
    //不符合单帧间角度变化的最小阈值
    continueNumber = (continueNumber - 1) >= 0 ? (continueNumber - 1) : 0;
  }
  //利用运动计算方向,备用方案,防止因防护看不到左右装甲的单侧灯条

  tuoluo newGet;  //本次信息
  newGet.angle = angle;
  newGet.tx = BestArmor.tx;
  newGet.direction = 1;

  float runDirection = newGet.tx - oldGed.tx;  //存储x坐标变化,用于判断运动状态
  float angleDirection =
      newGet.angle - oldGed.angle;  //存储角度变化,用于判断运动状态

  //利用tx坐标确定当前运动状态
  if (fabs(runDirection) <= MIN_RUN_DISTANCE) {  //两次tx移动距离小于规定最小值

    status = STILL;
    old_direction =
        TopRunDirection((old_direction + 1) % 2);  //扭腰切换方向概率较大
    newGet.direction = oldGed.direction;           //方向默认与上次相同

  } else if (fabs(runDirection) >=
             MAX_RUN_DISTANCE) {  //两次tx移动距离小于规定最大值

    status = SWITCH;
    newGet.direction = -oldGed.direction;  //装甲左右方向变向

  } else if (runDirection > 0) {  //两次tx移动距离符合范围,且向右运动

    status = RUN;
    old_direction = RIGHT;
    // tx变大,向右移动

    //记录运动方向
    if (isAngleEffective) {
      rightTimes++;
    }

    //判断角度大小变化关系,确定当前目标装甲是在左侧还是右侧
    if (angleDirection > 0) {
      newGet.direction = -1;
    } else {
      newGet.direction = 1;
    }

  } else {  //两次tx移动距离符合范围,且向左运动

    status = RUN;
    old_direction = LEFT;
    // tx变小向左移动

    //记录运动方向
    if (isAngleEffective) {
      leftTimes++;
    }
    //判断角度大小变化关系,确定当前目标装甲是在左侧还是右侧
    if (angleDirection > 0) {  //角度变大
      newGet.direction = 1;
    } else {  //角度变小
      newGet.direction = -1;
    }
  }

  angle = angle * newGet.direction;  //控制左右方向,添加正负
  //累计得到最大角度和最小角度,当前记录的是整个运动中的极值,可能存在问题,具体测再解决,可考虑更改模式为遍历vector容器方式,需要改很多
  if (angle > max_angle) {
    max_angle = angle;
  } else if (angle < min_angle) {
    min_angle = angle;
  }

  //记录陀螺位置点
  if (getPointTimes > LEASTFITTIMES) {  //当前已满足存储数量,开始更新
    circlePoints[getPointTimes % LEASTFITTIMES] =
        cv::Point2f(BestArmor.tx, BestArmor.tz);
  } else {
    circlePoints.push_back(cv::Point2f(
        BestArmor.tx,
        BestArmor.tz));  //存入circlePoints,用于轨迹拟合,得到摆动范围
  }
  //记录陀螺角度值,利用方差确定是否为陀螺
  if (getPointTimes >= SEC_MODE_SAVE_RUN_TIMES) {
    angles[getPointTimes % SEC_MODE_SAVE_RUN_TIMES] = angle;
  } else {
    angles[getPointTimes] = angle;
  }
  //累计次数加1
  getPointTimes++;
  //计算方差
  if (getPointTimes >= SEC_MODE_SAVE_RUN_TIMES) {
    float sum = 0;  //和
    //求和
    for (int i = 0; i < SEC_MODE_SAVE_RUN_TIMES; i++) {
      sum += angles[i];
    }
    float ave = sum / SEC_MODE_SAVE_RUN_TIMES;  //平均数
    float varSum = 0;                           //方差和
    //求方差
    for (int i = 0; i < SEC_MODE_SAVE_RUN_TIMES; i++) {
      varSum += pow(angles[i] - ave, 2);
    }
    if (varSum / SEC_MODE_SAVE_RUN_TIMES > MIN_VARIANCE) VarianceIsBig = true;
    cout << "方差计算结果:" << varSum / SEC_MODE_SAVE_RUN_TIMES << endl;
  }

  //判断是否更新oldGed变量,确认当前目标为陀螺则不更新,否则更新
  //最小二乘法拟合水平距离
  if (circlePoints.size() > 10 && continueNumber >= TOP_CONTINUE_NUM_MIN &&
      VarianceIsBig) {
    //当前为陀螺或扭腰状态
    if (fabs(leftTimes - rightTimes) <
        4) {  //记录左右转次数相差不大,则当前可能为扭腰
      //可能为扭腰
      direction = old_direction;
    } else {  //相差较大则为陀螺的概率较大
      //可能为陀螺
      if (leftTimes > rightTimes) {
        direction = LEFT;
      } else {
        direction = RIGHT;
      }
    }

    double circle_x, circle_y, radius;
    circleLeastFit(circlePoints, circle_x, circle_y, radius);  //最小二乘法拟合
                                                               //绘制

    cv::Mat drawImage = cv::Mat::zeros(500, 500, CV_8UC3);
    Draw(circlePoints, drawImage, cv::Scalar(255, 255, 255));
    circle(drawImage,
           cv::Point2f(10 * (circle_x + 20),
                       drawImage.rows / 2 + (circle_y - circlePoints[0].y)),
           10 * radius, cv::Scalar(255, 0, 0), 1, 4);
    imshow("陀螺绘制", drawImage);

    double porpation =
        sin(max_angle * CV_PI / 180) /
        (sin(max_angle * CV_PI / 180) + sin(-min_angle * CV_PI / 180));
    R = (2 * radius * porpation) / sin(max_angle * CV_PI / 180);
    //计算圆心左侧距离
    float leftDistance =
        2 * radius * sin(-min_angle * CV_PI / 180) /
        (sin(max_angle * CV_PI / 180) + sin(-min_angle * CV_PI / 180));
    //计算圆心右侧距离
    float rightDistance =
        2 * radius * sin(max_angle * CV_PI / 180) /
        (sin(max_angle * CV_PI / 180) + sin(-min_angle * CV_PI / 180));
    //计算圆心位置
    float center_tx = circle_x + (leftDistance - rightDistance) / 2.0;
    //暂时没找到好方法,先随便找一个点算一下,运动圆心的tz必定大于轨迹上tz,因而直接加上差
    float center_tz =
        sqrt(R * R - pow(fabs(center_tx - circlePoints[10].x), 2)) +
        circlePoints[10].y;
    center = cv::Point2f(center_tx, center_tz);

    cout << "min:" << min_angle << "  max:" << max_angle << endl;
    cout << "最小二乘得中心点:x:" << circle_x
         << "  R:" << radius / sin(max_angle * CV_PI / 180) << endl;

    //计算角速度
    if (status == SWITCH) {
      //当前切换了目标
      if (direction == LEFT) {  //当前左转
        if (newGet.angle - oldGed.angle > 0) {
          spinSpeed = newGet.angle - 90 - oldGed.angle;
        } else {
          //此情况时可能对于切换判断有误
          spinSpeed = newGet.angle - oldGed.angle;
        }
      } else {  //当前右转
        if (newGet.angle - oldGed.angle < 0) {
          spinSpeed = newGet.angle + 90 - oldGed.angle;
        } else {
          //此情况时可能对于切换判断有误
          spinSpeed = newGet.angle - oldGed.angle;
        }
      }
    } else if (status == RUN) {
      //同目标运动
      spinSpeed = newGet.angle - oldGed.angle;

    } else {
      //两次位置差别不大,状态判断为静止
      spinSpeed = oldSpinSpeed;
    }
    oldSpinSpeed = spinSpeed;  //更新变量
    oldGed = newGet;           //更新变量
    //间隔状态保留
    old_center = center;
    old_direction = direction;
    old_Speed = spinSpeed;
    old_status = status;
    old_result = true;
    return true;
  } else {
    //当前不是陀螺,更新oldGed
    oldGed = newGet;  //更新变量
    //间隔状态保留
    old_center = center;
    old_direction = direction;
    old_Speed = spinSpeed;
    old_status = status;
    old_result = false;
    return false;
  }
}

/**
 * @brief ShootTuoluo::getAngle
 * 计算当前装甲状态对应角度大小,无正负关系,所得结果均为正
 *                  计算过程利用小孔成像原理,首先测得一个标准值,该值包含距离和装甲宽度,
 *                  利用标准值和当前测的的目标距离,计算目标所应具备的正对时装甲宽度.利用
 *                  反余弦函数计算倾斜角度(具体详见华北理工大学2020赛季设计报告第四章
 * @param BestArmor                                传入的装甲识别结果
 * @return 角度大小
 */
double ShootTuoluo::getAngle(armor::Armor_Data BestArmor) {
  //    double width =
  //    GetDistance(BestArmor.left_light.center,BestArmor.right_light.center);
  double width =
      fabs(BestArmor.left_light.center.x - BestArmor.right_light.center.x);
  cout << "width:" << width << endl;
  double ledAngle;
  if (BestArmor.left_light.angle > 90)
    ledAngle = 180 - BestArmor.left_light.angle;
  else
    ledAngle = BestArmor.left_light.angle;
  if (BestArmor.right_light.angle > 90)
    ledAngle += 180 - BestArmor.right_light.angle;
  else
    ledAngle += BestArmor.right_light.angle;
  ledAngle /= 2.0;
  double armorAngle = atan2(
      fabs(BestArmor.right_light.center.y - BestArmor.left_light.center.y),
      BestArmor.right_light.center.x - BestArmor.left_light.center.x);
  double Angle = fabs(
      armorAngle -
      ledAngle * CV_PI /
          180);  //得到弧度差角,利用装甲倾斜角度和灯条倾斜角度关系弥补因俯仰角或是倾斜产生的计算误差
                 //    width *= cos(Angle);
  //    //得到用于计算的装甲宽度,因为旋转造成的视角不同从而使2d图像的装甲宽度不同
  //     width *= cos(armorAngle);
  double distance = sqrt(pow(BestArmor.tz, 2) + pow(BestArmor.tx, 2));
  double _cos = width / (tz_armor_width_ * (tz_normal / distance));
  cout << "cos:" << _cos << "distance:" << distance << endl;
  if (_cos >= 1) return 0;
  return acos(_cos) * 180 / CV_PI;
}

/**
 * @brief ShootTuoluo::drawImage                    估算并绘制陀螺中心
 * @param Src
 * @param angle                     当前装甲相对中心角度
 * @param object                    当前装甲坐标(tx,ty)
 * @param R                             陀螺估算半径
 * @param armorWidth        装甲正对状态下估计宽度
 */
void ShootTuoluo::drawImage(cv::Mat Src, double angle, cv::Point2f object,
                            double R, float armorWidth) {
  double adjustWidth = armorWidth * (R / 13.5) * sin(angle * CV_PI / 180);
  circle(Src, object + cv::Point2f(adjustWidth, 0), 20, cv::Scalar(0, 155, 255),
         -1, 12);
}

//计算空间两点距离
double GetSpaceDistance(cv::Point3f a, cv::Point3f b) {
  return sqrt(pow(a.x - b.x, 2.0) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

//计算空间三点圆心
cv::Point3f GetSpaceCenter(cv::Point3f a, cv::Point3f b, cv::Point3f c) {
  double x1 = a.x, y1 = a.y, z1 = a.z;
  double x2 = b.x, y2 = b.y, z2 = b.z;
  double x3 = c.x, y3 = c.y, z3 = c.z;

  double A1 = y1 * z2 - y1 * z3 - z1 * y2 + z1 * y3 + y2 * z3 - y3 * z2;
  double B1 = -x1 * z2 + x1 * z3 + z1 * x2 - z1 * x3 - x2 * z3 + x3 * z2;
  double C1 = x1 * y2 - x1 * y3 - y1 * x2 + y1 * x3 + x2 * y3 - x3 * y2;
  double D1 = -x1 * y2 * z3 + x1 * y3 * z2 + x2 * y1 * z3 - y1 * x3 * z2 -
              x2 * y3 * z1 + x3 * y2 * z1;

  double A2 = 2 * (x2 - x1);
  double B2 = 2 * (y2 - y1);
  double C2 = 2 * (z2 - z1);
  double D2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;

  double A3 = 2 * (x3 - x1);
  double B3 = 2 * (y3 - y1);
  double C3 = 2 * (z3 - z1);
  double D3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;

  double mo = A1 * B2 * C3 + A2 * B3 * C1 + B1 * C2 * A3 - C1 * B2 * A3 -
              C2 * B3 * A1 - B1 * A2 * C3;

  double x = -((B2 * C3 - B3 * C2) * D1 + (-(B1 * C3 - C1 * B3)) * D2 +
               (B1 * C2 - C1 * B2) * D3) /
             mo;
  double y = -(-(A2 * C3 - C2 * A3) * D1 + (A1 * C3 - C1 * A3) * D2 +
               (-(A1 * C2 - A2 * C1)) * D3) /
             mo;
  double z = -((A2 * B3 - B2 * A3) * D1 + (-(A1 * B3 - A3 * B1)) * D2 +
               (A1 * B2 - A2 * B1) * D3) /
             mo;

  return cv::Point3f(x, y, z);
}

// 2d三点拟合得到圆心
cv::Point2f GetSpaceCenter(cv::Point2f point_one, cv::Point2f point_two,
                           cv::Point2f point_three) {
  // x1，y1为box_buff中心点，x2，y2为last_box_buff中心点
  float x1 = point_one.x;
  float x2 = point_two.x;
  float x3 = point_three.x;
  float y1 = point_one.y;
  float y2 = point_two.y;
  float y3 = point_three.y;

  //******************利用三点求圆心************
  float A1 = 2 * (x2 - x1);
  float B1 = 2 * (y2 - y1);
  float C1 = x2 * x2 + y2 * y2 - x1 * x1 - y1 * y1;
  float A2 = 2 * (x3 - x2);
  float B2 = 2 * (y3 - y2);
  float C2 = x3 * x3 + y3 * y3 - x2 * x2 - y2 * y2;
  float X = ((C1 * B2) - (C2 * B1)) / ((A1 * B2) - (A2 * B1));
  float Y = ((A1 * C2) - (A2 * C1)) / ((A1 * B2) - (A2 * B1));
  return cv::Point2f(X, Y);
  //**********************************************
}

/**
 * @brief circleLeastFit 最小二乘法拟合圆,得半径,此方法类似于圆形拟合
 * @param points                            所记录点集
 * @param center_x                       中心x坐标
 * @param center_y                      中心y坐标
 * @param radius                            拟合圆半径
 * @return
 */
bool circleLeastFit(const std::vector<cv::Point2f> &points, double &center_x,
                    double &center_y, double &radius) {
  center_x = 0.0f;
  center_y = 0.0f;
  radius = 0.0f;
  if (points.size() < 3) {
    return false;
  }

  double sum_x = 0.0f, sum_y = 0.0f;
  double sum_x2 = 0.0f, sum_y2 = 0.0f;
  double sum_x3 = 0.0f, sum_y3 = 0.0f;
  double sum_xy = 0.0f, sum_x1y2 = 0.0f, sum_x2y1 = 0.0f;

  int N = points.size();
  for (int i = 0; i < N; i++) {
    double x = points[i].x;
    double y = points[i].y;
    double x2 = x * x;
    double y2 = y * y;
    sum_x += x;
    sum_y += y;
    sum_x2 += x2;
    sum_y2 += y2;
    sum_x3 += x2 * x;
    sum_y3 += y2 * y;
    sum_xy += x * y;
    sum_x1y2 += x * y2;
    sum_x2y1 += x2 * y;
  }

  double C, D, E, g, H;
  double a, b, c;

  C = N * sum_x2 - sum_x * sum_x;
  D = N * sum_xy - sum_x * sum_y;
  E = N * sum_x3 + N * sum_x1y2 - (sum_x2 + sum_y2) * sum_x;
  g = N * sum_y2 - sum_y * sum_y;
  H = N * sum_x2y1 + N * sum_y3 - (sum_x2 + sum_y2) * sum_y;
  a = (H * D - E * g) / (C * g - D * D);
  b = (H * C - E * D) / (D * D - g * C);
  c = -(a * sum_x + b * sum_y + sum_x2 + sum_y2) / N;

  center_x = a / (-2);
  center_y = b / (-2);
  radius = sqrt(a * a + b * b - 4 * c) / 2;
  return true;
}

/**
 * @brief Draw                  利用小孔成像原理估算陀螺圆心
 * @param object
 * @param src
 * @param a
 */
void Draw(vector<cv::Point2f> object, cv::Mat src, cv::Scalar a) {
  for (size_t i = 0; i < object.size(); i++) {
    circle(src,
           cv::Point2f((object[i].x + 20) * 10,
                       src.rows / 2 + (object[i].y - object[0].y) * 10),
           5, a, -1, 4);
  }
}
