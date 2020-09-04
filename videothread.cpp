#include "videothread.h"

#include <QDebug>


VideoThread::VideoThread()
{

}

VideoThread::~VideoThread()
{

}

void VideoThread::process()
{
    cv::Mat frame_cv;
    cv::Mat frame_to_resize;
    cv::Mat frame_denoised;
    cv::Mat frame_gray;
    cv::Mat frame_blur;
    cv::Mat frame_canny;

    if(!cv_camera->read(frame_to_resize))
    {
        emit error("Camera read error");
        emit finished();
    }

    if(!frame_to_resize.empty())
    {

        cv::resize(frame_to_resize, frame_cv, cv::Size(1280,960));
        cv::cvtColor(frame_cv, frame_gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(frame_gray, frame_blur, cv::Size(5,5),1);
        cv::Canny(frame_blur,frame_canny,60,100);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(frame_canny(cv::Rect(0,0,100,180)), contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        cv::Point cv_robot_origin;

        if(!contours.empty())
        {
            cv_robot_origin = contours[0][0];
            cv::circle(frame_cv, contours[0][0], 3, cv::Scalar(255,0,0),1);
            cv::putText(frame_cv, "(0,0)", cv::Point(contours[0][0].x+5, contours[0][0].y-5), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
        }
        else
            cv_robot_origin = cv::Point(0,0);

        QImage frame_qt = MatToQImage(frame_cv);
        emit readyImg(frame_qt, cv_robot_origin.x, cv_robot_origin.y);
    }

}

void VideoThread::start()
{
    cv_camera = new cv::VideoCapture;

    if(cv_camera->open(0, cv::CAP_ANY))
    {
        emit cameraOpened();
        cv_camera->set(cv::CAP_PROP_FRAME_WIDTH, resolution_max_width);
        cv_camera->set(cv::CAP_PROP_FRAME_HEIGHT, resolution_max_height);
        cv_camera->set(cv::CAP_PROP_CONTRAST, 10);
        cv_camera->set(cv::CAP_PROP_FOCUS, 100);
        cv_camera->set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
        cv_camera->set(cv::CAP_PROP_SATURATION, 20);
        cv_camera->set(cv::CAP_PROP_BRIGHTNESS, -3);
        timer = new QTimer;
        connect(timer, &QTimer::timeout, this, &VideoThread::process);
        timer->start(1);
    }
    else
    {
        emit error("No device connected!");
        emit finished();
    }
}

QImage VideoThread::MatToQImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    if(mat.type()==CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    return QImage();
}
