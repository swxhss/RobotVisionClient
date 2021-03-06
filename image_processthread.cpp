#include "image_processthread.h"
#include <QDebug>
#include <vector>
#include <iostream>
using namespace cv;
using namespace  std;
Image_processThread::Image_processThread()
{
    leftCameraIndex = 0;
    rightCameraIndex = 1;
    R_Gen = 1.0;
    G_Gen = 1.0;
    B_Gen = 1.0;
    Contrast_Gen = 100;
    Bright_Gen = 0;
    AutoWhiteBalance = false;
    leftCameraStatus = CAMERA_OPEN;
    rightCameraStatus = CAMERA_OPEN;

}

void Image_processThread::accept_leftdeviceIndex(int index)
{
    leftCameraIndex = index;
}

void Image_processThread::accept_rightdeviceIndex(int index)
{
    rightCameraIndex = index;
}

void Image_processThread::accept_deviceNum(int usrCameraNum)
{
    deviceNum = usrCameraNum;
}

void Image_processThread::accept_closeLeftCamera()
{
    QMutexLocker locker(&m_lock);
    leftCameraStatus = CAMERA_CLOSE;

}

void Image_processThread::accept_closeRightCamera()
{
    //   m_RightCamera->release();
    rightCameraStatus = CAMERA_CLOSE;
}

void Image_processThread::accept_RGBGen(int r, int g, int b)
{
    R_Gen += double(r)/100;
    G_Gen += double(g)/100;
    B_Gen += double(b)/100;
}

void Image_processThread::accept_ContrastBrightGen(int contrast, int bright)
{
    Contrast_Gen += contrast;
    Bright_Gen   += bright;
}

void Image_processThread::accept_AutoWhiteBalance()
{
    AutoWhiteBalance = true;
}

void Image_processThread::accept_RobotMode(int robotMode)
{
    qDebug()<<"机器人模式"<<robotMode<<endl;
    switch (robotMode) {
    case 1:
        robotmode = Road_RobotMode;
        break;
    case 2:
        robotmode = Basket_RobotMode;
        break;
    default:
        break;
    }
}

QImage Image_processThread::convertMatToQImage(Mat &mat)
{
    QImage img;
    int nChannel=mat.channels();
    if(nChannel==3)
    {
        cvtColor(mat,mat,CV_BGR2RGB);
        img = QImage((const unsigned char*)mat.data,mat.cols,mat.rows,QImage::Format_RGB888);
    }
    else if(nChannel==4)
    {
        img = QImage((const unsigned char*)mat.data,mat.cols,mat.rows,QImage::Format_ARGB32);
    }
    else if(nChannel==1)
    {
        img = QImage((const unsigned char*)mat.data,mat.cols,mat.rows,QImage::Format_Indexed8);
    }

    return img;
}

Mat Image_processThread::contrastAndBrightSet(Mat &frame, int contrastValue, int BrightValue)
{
    Mat preset_frame;
    preset_frame = frame.clone();
    for(int y =0;y<frame.rows;y++)
    {
        for(int x =0;x<frame.cols;x++)
        {
            for(int c=0;c<3;c++)
            {
                preset_frame.at<Vec3b>(y,x)[c] = saturate_cast<uchar>((contrastValue*0.01)*frame.at<Vec3b>(y,x)[c]+BrightValue);
            }
        }
    }

    return preset_frame;
}

int Image_processThread::OSTU_Threshold(Mat &frame)
{
    int maxT = 0;//阈值
    float devi = 0;//方差
    float maxDevi = 0;
    //计算直方图需要的参数
    MatND dsthist;
    int hist_size = 256;//直方图大小
    float hranges[] = { 0,255 };//直方图范围
    const float *ranges[] = { hranges };
    int dims = 1;//输入数组个数
    int channels = 0;
    calcHist(&frame,1,&channels,Mat(),dsthist,dims,&hist_size,ranges);
    float GrayArray[256] = { 0 };
    for (int i = 0; i < 256; i++)
    {
        GrayArray[i] = dsthist.at<float>(i);
    }
    for (int t = 0; t < 256; t++)
    {
        float count = 0;
        float average1 = 0;//m1
        float probability1 = 0;//p1
        float average2 = 0;//m2
        float probability2 = 0;//p1
        int j = 0;
        for (; j <= t; j++)
        {
            average1 += j*GrayArray[j];
            count    += GrayArray[j];
        }
        average1 = average1 / count;
        probability1 = count / (frame.cols*frame.rows);
        for (j = t + 1; j < 256; j++)
        {
            average2 += j*GrayArray[j];
        }
        average2 = average2 / (frame.cols*frame.rows - count);
        probability2 = 1 - probability1;
        //类间方差计算方法p1*p2*(m1-m2)(m1-m2)
        devi = probability1*probability2*(average2 - average1)*(average2-average1);
        if (devi > maxDevi)
        {
            maxDevi = devi;
            maxT = t;
        }
    }
    return maxT;
}

void Image_processThread::PaintHist(Mat &gray_frame)
{
    MatND dsthist;
    int dims = 1;
    float hranges[] = { 0,255 };
    const float *ranges[] = { hranges };
    int size = 256;
    int channels = 0;
    calcHist(&gray_frame,1,&channels,Mat(),dsthist,dims,&size,ranges);
    int scale = 1;
    Mat hist_frame(size* scale,size,CV_8U,Scalar(0));
    double minValue = 0;
    double maxValue = 0;
    minMaxLoc(dsthist,&minValue,&maxValue,0,0);
    int hpt = saturate_cast<int>(0.9*size);
    for (int i = 0; i < 256; i++)
    {
        float binValue = dsthist.at<float>(i);
        int realValue = saturate_cast<int>(binValue*hpt / maxValue);
        rectangle(hist_frame,Point(i*scale,size-1),Point((i+1)*scale-1,size-realValue),Scalar(255));
    }
    imshow("hist_frame",hist_frame);
}

Mat Image_processThread::LineDetect(Mat &binary_frame,Mat &bilater_frame)
{
    Mat canny_frame;
    Canny(binary_frame,canny_frame,ostu_threshlodValue,ostu_threshlodValue*3,3);
    vector<Vec4i> lines;
    HoughLinesP(canny_frame,lines,1,CV_PI/180, 0,50,10);
    cout << "line num" << lines.size() << endl;
    for (size_t i = 0; i < lines.size(); i++)
    {
        Vec4i l = lines[i];
        line(bilater_frame,Point(l[0],l[1]),Point(l[2],l[3]),Scalar(0, 255, 0),9,LINE_AA);
    }
    return bilater_frame;
}

Mat Image_processThread::ThresholdProcess(Mat &bilater_frame)
{
    Mat gray_frame,binary_frame;
    cvtColor(bilater_frame,gray_frame,COLOR_BGR2GRAY);
    ostu_threshlodValue = OSTU_Threshold(gray_frame);
    threshold(gray_frame,binary_frame,ostu_threshlodValue,255,THRESH_BINARY);
    return binary_frame;
}
//导向滤波
Mat Image_processThread::guidedfilter(Mat &srcImage, Mat &srcClone, int r, double eps)
{
    srcImage.convertTo(srcImage,CV_64FC1);
        srcClone.convertTo(srcClone, CV_64FC1);
        int nRows = srcImage.rows;
        int nCols = srcImage.cols;
        Mat boxResult;
        //1.计算均值
        boxFilter(Mat::ones(nRows,nCols,srcImage.type),boxResult,CV_64FC1,Size(r,r));
        //2 生成导向均值mean_I
        Mat mean_I;
        boxFilter(srcImage, mean_I, CV_64FC1, Size(r, r));
        //3 生成原始均值mean_P
        Mat mean_P;
        boxFilter(srcClone,mean_P,CV_64FC1,Size(r,r));
        //4 生成互相关均值mean_IP
        Mat mean_IP;
        boxFilter(srcImage.mul(srcClone),mean_IP,CV_64FC1,Size(r,r));
        Mat cov_IP = mean_IP - mean_I.mul(mean_P);
        //5 生成自相关均值mean_II
        Mat mean_II;
        boxFilter(srcImage.mul(srcImage),mean_II,CV_64FC1,Size(r,r));
        //6 计算相关系数
        Mat var_I = mean_II - mean_I.mul(mean_I);
        Mat var_II = mean_IP - mean_I.mul(mean_P);
        //7 计算参数系数a,b
        Mat a = cov_IP / (var_I + eps);
        Mat b = mean_P - a.mul(mean_I);
        //8 计算参数系数a,b的均值
        Mat mean_a;
        boxFilter(a,mean_a,CV_64FC1,Size(r,r));
        mean_a = mean_a / boxResult;
        Mat mean_b;
        boxFilter(b,mean_b,CV_64FC1,Size(r,r));
        mean_b = mean_b / boxResult;
        //9 生成输出矩阵
        Mat resultMat = mean_a.mul(srcImage) + mean_b;
        return resultMat;
}



void Image_processThread::run()
{
    switch (deviceNum) {
    case LEFT_CAMERA:
        leftCameraStatus = CAMERA_OPEN;
        m_leftCamera = new VideoCapture(leftCameraIndex);
            while(1)
            {
                QMutexLocker locker(&m_lock);
                if(leftCameraStatus ==CAMERA_CLOSE)
                    return;
                if(!AutoWhiteBalance)
                {

                    Mat frame,preset_frame,bilater_frame,binary_frame,dst_frame,binary_dst_frame,resize_frame,resize_binary_frame;vector<Mat>imageBGR;
                    m_leftCamera->operator >>(frame);
                    /**/
                    //TODO 图像处理区域
                    split(frame,imageBGR);
                    imageBGR[0] = imageBGR[0]*B_Gen;
                    imageBGR[1] = imageBGR[1]*G_Gen;
                    imageBGR[2] = imageBGR[2]*R_Gen;
                    merge(imageBGR,frame);
                    preset_frame= contrastAndBrightSet(frame,Contrast_Gen,Bright_Gen);
                    //双边滤波器
                    bilateralFilter(preset_frame,bilater_frame,10,10*2,10/2);
                    //阈值灰度二值化处理
                    binary_frame = ThresholdProcess(bilater_frame);
                    switch (robotmode) {
                    case Road_RobotMode:
                        //直线检测
                        bilater_frame = LineDetect(binary_frame,bilater_frame);
                        break;
                    default:
                        break;
                    }

                    /**/
                    /**********/
                    //TODO opencv to qt显示
                    dst_frame = bilater_frame.clone();
                    binary_dst_frame = binary_frame.clone();
                    resize(dst_frame,resize_frame,Size(320,240),CV_INTER_AREA);
                    resize(binary_dst_frame,resize_binary_frame,Size(320,240),CV_INTER_AREA);
                    left_frame  = convertMatToQImage(resize_frame);
                    right_frame = convertMatToQImage(resize_binary_frame);
                    send_leftdispframe(left_frame);
                    send_rightdispframe(right_frame);
                    /**********/
                    /**********/
                    //TODO 图像测试处理区域
                    /**********/
                    imshow("left_video",bilater_frame);
                    waitKey(30);
                }
                else
                {
                    AutoWhiteBalance = !AutoWhiteBalance;
                    Mat frame,preset_frame,bilater_frame,binary_frame,dst_frame,binary_dst_frame,resize_frame,resize_binary_frame;
                    vector<Mat>imageBGR;
                    m_leftCamera->operator >>(frame);
                    /**/
                    //TODO 图像处理区域
                    split(frame,imageBGR);
                    double R_BalanceValue, G_BalanceValue, B_BalanceValue;
                    B_BalanceValue = mean(imageBGR[0])[0];
                    G_BalanceValue = mean(imageBGR[1])[0];
                    R_BalanceValue = mean(imageBGR[2])[0];
                    double KR, KG, KB;
                    KB = (R_BalanceValue + G_BalanceValue + B_BalanceValue) / (3 * B_BalanceValue);
                    KG = (R_BalanceValue + G_BalanceValue+ B_BalanceValue) / (3 * G_BalanceValue);
                    KR = (R_BalanceValue + G_BalanceValue + B_BalanceValue) / (3 * R_BalanceValue);
                    R_Gen =  KR;
                    G_Gen =  KG;
                    B_Gen =  KB;
                    imageBGR[0] = imageBGR[0]*KB;
                    imageBGR[1] = imageBGR[1]*KG;
                    imageBGR[2] = imageBGR[2]*KR;
                    merge(imageBGR,frame);
                    preset_frame= contrastAndBrightSet(frame,Contrast_Gen,Bright_Gen);
                    //双边滤波检测
                    bilateralFilter(preset_frame,bilater_frame,10,10*2,10/2);
                    //阈值灰度二值化处理
                    binary_frame = ThresholdProcess(bilater_frame);
                    switch (robotmode) {
                    case Road_RobotMode:
                        //直线检测
                        bilater_frame = LineDetect(binary_frame,bilater_frame);
                        break;
                    default:
                        break;
                    }
                    /**/
                    /**********/
                    //TODO opencv to qt显示
                    dst_frame = bilater_frame.clone();
                    binary_dst_frame = binary_frame.clone();
                    resize(dst_frame,resize_frame,Size(320,240),CV_INTER_AREA);
                    resize(binary_dst_frame,resize_binary_frame,Size(320,240),CV_INTER_AREA);
                    left_frame  = convertMatToQImage(resize_frame);
                    right_frame = convertMatToQImage(resize_binary_frame);
                    send_leftdispframe(left_frame);
                    send_rightdispframe(right_frame);
                    /**********/
                    /**********/
                    //TODO 图像测试处理区域

                    /**********/
                    imshow("left_video",bilater_frame);
                    waitKey(30);
                }
            }
            m_leftCamera->release();
        break;
    case RIGHT_CAMERA:
        m_RightCamera = new VideoCapture(rightCameraIndex);
        while(1)
        {
            Mat frame,dst_frame,resize_frame;
            m_RightCamera->operator >>(frame);
            dst_frame = frame.clone();
            resize(dst_frame,resize_frame,Size(320,240),CV_INTER_AREA);
            right_frame = convertMatToQImage(resize_frame);
            send_rightdispframe(right_frame);
            imshow("right_video",frame);
            waitKey(30);

        }
        break;
    case ALL_CAMERA:
        m_leftCamera = new VideoCapture(leftCameraIndex);
        m_RightCamera = new VideoCapture(rightCameraIndex);
        while(1)
        {
            Mat left_ori_frame,right_ori_frame,left_dst_frame,right_dst_frame,left_resize_frame,right_resize_frame,mix_frame;
            m_leftCamera->operator >>(left_ori_frame);
            m_RightCamera->operator >>(right_ori_frame);
            left_dst_frame = left_ori_frame.clone();
            right_dst_frame = right_ori_frame.clone();
            resize(left_dst_frame,left_resize_frame,Size(320,240),CV_INTER_AREA);
            resize(right_dst_frame,right_resize_frame,Size(320,240),CV_INTER_AREA);
            left_frame = convertMatToQImage(left_resize_frame);
            right_frame = convertMatToQImage(right_resize_frame);
            send_alldispframe(left_frame,right_frame);
            /**********/
            //TODO 图像处理区域
            addWeighted(left_ori_frame,0.8,right_ori_frame,0.2,0,mix_frame);
            /**********/
            imshow("mix_frame",mix_frame);
            waitKey(30);
        }
        break;
    default:
        break;
    }
}
