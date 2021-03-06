#ifndef IMAGE_PROCESSTHREAD_H
#define IMAGE_PROCESSTHREAD_H
#include <QObject>
#include <QThread>
#include <QImage>
#include <opencv.hpp>
#include <vector>
#include <QMutex>
#include <QMutexLocker>
#define LEFT_CAMERA  1
#define RIGHT_CAMERA 2
#define ALL_CAMERA   3
#define CAMERA_OPEN  4
#define CAMERA_CLOSE  5
enum RobotMode
{
    Road_RobotMode=1,
    Basket_RobotMode
};
using namespace cv;
class Image_processThread : public QThread
{
    Q_OBJECT
public:
   Image_processThread();
   //test
public:
   int leftCameraIndex;//左相机设备索引
   int rightCameraIndex;//右相机设备索引
   int deviceNum;//相机使用计数，单目or双目
   double R_Gen;//R通道增益
   double G_Gen;//G通道增益
   double B_Gen;//B通道增益
   int Contrast_Gen;
   int Bright_Gen;
   int ostu_threshlodValue;//自适应阈值
   bool AutoWhiteBalance;
   int leftCameraStatus;
   bool  rightCameraStatus;
   enum RobotMode robotmode;
   VideoCapture *m_leftCamera;
   VideoCapture *m_RightCamera;
   QImage left_frame;
   QImage right_frame;
   QMutex m_lock;
signals:
   void imageCapturesignal();
   void send_leftdispframe(QImage);
   void send_rightdispframe(QImage);
   void send_alldispframe(QImage,QImage);
public slots:
   void accept_leftdeviceIndex(int index);
   void accept_rightdeviceIndex(int index);
   void accept_deviceNum(int);
   void accept_closeRightCamera();
   void accept_RGBGen(int,int,int);
   void accept_ContrastBrightGen(int,int);
   void accept_AutoWhiteBalance();
   void accept_RobotMode(int);
public:
   void accept_closeLeftCamera();
   QImage convertMatToQImage(Mat &mat);
   Mat    contrastAndBrightSet(Mat &frame,int contrastValue,int BrightValue);
   int    OSTU_Threshold(Mat &frame);//OSUT找全局最优阈值为后面的二值化处理做预备
   void   PaintHist(Mat &gray_frame);//灰度直方图绘制
   Mat    LineDetect(Mat &binary_frame,Mat &bilater_frame);//直线检测
   Mat    ThresholdProcess(Mat &bilater_frame);
protected:
    void run();
};
#endif // IMAGE_PROCESSTHREAD_H
