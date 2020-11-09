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
        if(detect)
        {
            cv::cvtColor(frame_cv, frame_gray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(frame_gray, frame_blur, cv::Size(5,5),0);
            cv::Canny(frame_blur,frame_canny_to_dilate,30,90);
            cv::Mat kernel = cv::Mat(4, 4, CV_8U, cv::Scalar(1));
            cv::dilate(frame_canny_to_dilate, frame_canny, kernel);

            std::vector<std::vector<cv::Point>> contours;
            std::vector<std::vector<cv::Point>> contours_2;
            std::vector<cv::Vec4i> hierarchy;
            std::vector<cv::Vec4i> hierarchy_2;
            cv::Point cv_robot_origin;
            cv::Point start;
            cv::Rect shape;
            cv::Rect org;
            cv::Mat bin;
            cv::Mat eroded;
            cv::Mat eroded_2;
            cv::Mat dilated;
            cv::Mat kernel_2 = cv::Mat(1, 1, CV_8U, cv::Scalar(1));
            cv::Rect internal = cv::Rect(cv::Point((frame_cv.cols/2)-200,(frame_cv.rows/2)-200),cv::Point((frame_cv.cols/2)+200,(frame_cv.rows/2)+200));
            std::vector<cv::Point2f> corners;

            cv::Rect origin = cv::Rect(0,0,120,190);
            cv::findContours(frame_canny(origin), contours_2, hierarchy_2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);


            if(!contours_2.empty())
            {
                org = cv::boundingRect(contours_2[0]);
                cv_robot_origin = org.tl();
                cv::putText(frame_cv, "(0,0)", cv::Point(cv_robot_origin.x+6, cv_robot_origin.y-6), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
                cv::circle(frame_cv, cv_robot_origin, 3, cv::Scalar(0,0,255));
            }

            cv::threshold(frame_gray(internal), bin, 0, 255, cv::THRESH_OTSU+cv::THRESH_BINARY);
            cv::morphologyEx(bin, eroded_2, cv::MORPH_OPEN, kernel_2);
            cv::morphologyEx(eroded_2, eroded, cv::MORPH_CLOSE, kernel_2);
            cv::dilate(eroded, dilated, kernel_2);
            cv::findContours(dilated, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::namedWindow("b", cv::WINDOW_NORMAL);
            cv::imshow("b", dilated);

            if(!contours.empty())
            {
                for(std::vector<cv::Point>contour : contours)
                {
                    double area = cv::contourArea(contour);
                    if(area > 100)
                    {
                        cv::approxPolyDP(contour, corners, 0.01*cv::arcLength(contour,true), true);
                        shape = cv::boundingRect(corners);
                        //cv::rectangle(frame_cv, shape, cv::Scalar(255,255,0), 1);
                        start = cv::Point(shape.br().x+(frame_cv.cols/2)-200, shape.br().y+(frame_cv.rows/2)-200);
                        emit pcb_found();
                    }
                }
                cv::circle(frame_cv, start, 3, cv::Scalar(255,255,0),1);
                cv::putText(frame_cv, "Start", start, cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
            }

            emit positions(zoomed_origin, cv_robot_origin.x, cv_robot_origin.y, start.x, start.y, shape.width, shape.height);
        }
        else if(zoomed_origin)
        {
            cv::cvtColor(frame_cv, frame_gray, cv::COLOR_BGR2GRAY);

            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::Mat bin;
            cv::Mat eroded_open;
            cv::Mat eroded;


            cv::threshold(frame_gray, bin, 0, 255, cv::THRESH_OTSU+cv::THRESH_BINARY);

            cv::Mat kernel = cv::Mat(2, 2, CV_8SC1, cv::Scalar(1));

            cv::morphologyEx(bin, eroded, cv::MORPH_OPEN, kernel);
            cv::morphologyEx(bin, eroded, cv::MORPH_CLOSE, kernel);

            cv::findContours(eroded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

            cv::namedWindow("a", cv::WINDOW_NORMAL);
            cv::imshow("a",eroded);

            cv::RotatedRect shape;
            cv::Point2f points[4];
            std::vector<cv::Point2f> corners;


            if(!contours.empty())
            {
                for(std::vector<cv::Point>contour : contours)
                {
                    double area = cv::contourArea(contour);
                    if(area > 50000)
                    {
                        cv::approxPolyDP(contour, corners, 0.01*cv::arcLength(contour,true), true);
                        shape = cv::minAreaRect(corners);
                        shape.points(points);

                        pcb_origin = points[3];
                        cv::circle(frame_cv, cv::Point(pcb_origin.x, pcb_origin.y), 5, cv::Scalar(255,255,0),2);
                        cv::putText(frame_cv, "Corner", cv::Point(pcb_origin.x, pcb_origin.y-5), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
                        emit positions(1, pcb_origin.x, pcb_origin.y, 0, 0, 0, 0);
                    }
                }
            }
        }
        QImage frame_qt = MatToQImage(frame_cv);
        emit readyImg(frame_qt);
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
        cv_camera->set(cv::CAP_PROP_AUTOFOCUS, 0);
        //cv_camera->set(cv::CAP_PROP_EXPOSURE, -3);
        cv_camera->set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);
        cv_camera->set(cv::CAP_PROP_SATURATION, 20);
        cv_camera->set(cv::CAP_PROP_BRIGHTNESS, 3);
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

void VideoThread::start_detection(bool dct)
{
    detect = dct;
}

void VideoThread::scan_origin_detect(bool b)
{
    zoomed_origin = b;
}

void VideoThread::refocus(int fcs)
{
    cv_camera->set(cv::CAP_PROP_FOCUS, fcs);
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
