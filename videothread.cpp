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
    cv::Mat frame_trun;
    cv::Mat frame_to_resize;
    cv::Mat frame_denoised;
    cv::Mat frame_gray;
    cv::Mat frame_blur;
    cv::Mat frame_canny;
    cv::Mat frame_canny_to_dilate;

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
        cv::Canny(frame_blur,frame_canny_to_dilate,50,100);
        cv::Mat kernel = cv::Mat(5, 5, CV_8UC1, cv::Scalar(1));

        cv::dilate(frame_canny_to_dilate, frame_canny, kernel);

//        cv::namedWindow("a");
//        cv::imshow("a", frame_canny);
//        cv::namedWindow("b");
//        cv::imshow("b", frame_gray);


        std::vector<std::vector<cv::Point>> contours;
        std::vector<std::vector<cv::Point>> contours_2;
        std::vector<cv::Vec4i> hierarchy;
        std::vector<cv::Vec4i> hierarchy_2;
        cv::findContours(frame_canny, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        cv::findContours(frame_canny(cv::Rect(0,0,150,150)), contours_2, hierarchy_2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        cv::Point cv_robot_origin;
        cv::Point start;
        cv::Point stop;
        cv::Rect shape;
        if(!contours.empty() && !contours_2.empty())
        {
            for(std::vector<cv::Point>contour : contours)
            {
                double area = cv::contourArea(contour);
                if(area > 10000 && area < 800000)
                {
                    std::vector<cv::Point> ConvexHullPoints = contourConvexHull(contour);
                    //cv::polylines(frame_cv, ConvexHullPoints, true, cv::Scalar(255,0,0),2);
                    shape = cv::boundingRect(ConvexHullPoints);
                    cv::rectangle(frame_cv, shape.tl(), shape.br(), cv::Scalar(0,255,0),2);
                    start = cv::Point(shape.x,shape.y);
                    //stop = cv::Point(shape.x+shape.width, shape.y+shape.height);
                    frame_trun = frame_cv(shape);
                    //cv::drawContours(frame_cv, std::vector<std::vector<cv::Point>>(1,contour), -1, cv::Scalar(0,0,255), 2);
                }
            }
            cv::circle(frame_cv, start, 5, cv::Scalar(255,255,0),2);
            cv::circle(frame_cv, stop, 5, cv::Scalar(255,255,0),2);
            cv::putText(frame_cv, "Start", start, cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
            //cv::putText(frame_cv, "Stop", stop, cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
        }
        if(!contours_2.empty())
        {
            cv_robot_origin = contours_2[0][0];
            cv::putText(frame_cv, "(0,0)", cv::Point(cv_robot_origin.x+6, cv_robot_origin.y+6), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
            cv::circle(frame_cv, cv_robot_origin, 3, cv::Scalar(0,0,255));
        }

        QImage frame_qt = MatToQImage(frame_cv);
        emit readyImg(frame_qt);
        emit positions(cv_robot_origin.x, cv_robot_origin.y, start.x, start.y, shape.width, shape.height);
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
        cv_camera->set(cv::CAP_PROP_CONTRAST, -10);
        cv_camera->set(cv::CAP_PROP_FOCUS, 50);
        cv_camera->set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
        cv_camera->set(cv::CAP_PROP_SATURATION, 20);
        cv_camera->set(cv::CAP_PROP_BRIGHTNESS, 2);
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

std::vector<cv::Point> VideoThread::contourConvexHull(std::vector<cv::Point> contours)
{
    std::vector<cv::Point> result;
    std::vector<cv::Point> pts;

    for(size_t i=0; i<contours.size(); i++)
            pts.push_back(contours[i]);
    cv::convexHull(pts,result);
    return result;
}
