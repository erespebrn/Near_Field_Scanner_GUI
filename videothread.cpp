//#include "videothread.h"

//#include <QDebug>
//#include <QMessageBox>

//VideoThread::VideoThread()
//{

//}

//VideoThread::~VideoThread()
//{

//}

//void VideoThread::process()
//{
//    cv::Mat frame_cv;
//    cv::Mat frame_to_resize;
//    cv::Mat frame_gray;
//    cv::Mat frame_blur;
//    cv::Mat frame_canny;
//    cv::Mat frame_canny_to_dilate;

//    cv::Point start;
//    cv::Rect shape_1;

//    if(!cv_camera->read(frame_to_resize))
//    {
//        emit error("Camera read error");
//        emit finished();
//    }

//    if(!frame_to_resize.empty())
//    {
//        cv::resize(frame_to_resize, frame_cv, cv::Size(1280,960));
//        if(detect)
//        {
//            is_height_measured = false;
//            cv::cvtColor(frame_cv, frame_gray, cv::COLOR_BGR2GRAY);
//            cv::GaussianBlur(frame_gray, frame_blur, cv::Size(7,7),1);
//            cv::Canny(frame_blur,frame_canny_to_dilate,30,90);
//            cv::Mat kernel = cv::Mat(4, 4, CV_8U, cv::Scalar(1));
//            cv::dilate(frame_canny_to_dilate, frame_canny, kernel);

//            std::vector<std::vector<cv::Point>> contours;
//            std::vector<std::vector<cv::Point>> contours_2;
//            std::vector<cv::Vec4i> hierarchy;
//            std::vector<cv::Vec4i> hierarchy_2;

//            cv::Rect shape_1;
//            cv::Rect org;
//            cv::Mat bin;
//            cv::Mat eroded;
//            cv::Mat eroded_2;
//            cv::Mat dilated;
//            cv::Mat kernel_2 = cv::Mat(1, 1, CV_8UC1, cv::Scalar(1));
//            cv::Rect internal = cv::Rect(cv::Point((frame_cv.cols/2)-400,(frame_cv.rows/2)-200),cv::Point((frame_cv.cols/2)+400,(frame_cv.rows/2)+400));
//            std::vector<cv::Point2f> corners;


////            cv::Point cv_robot_origin;
////            cv::Rect origin = cv::Rect(0,0,120,190);
////            cv::findContours(frame_canny(origin), contours_2, hierarchy_2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);


////            if(!contours_2.empty())
////            {
////                org = cv::boundingRect(contours_2[0]);
////                cv_robot_origin = org.tl();
////                qDebug() << "X: " << cv_robot_origin.x << "Y: " << cv_robot_origin.y;
////                cv::putText(frame_cv, "(0,0)", cv::Point(cv_robot_origin.x+6, cv_robot_origin.y-6), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
////                cv::circle(frame_cv, cv_robot_origin, 3, cv::Scalar(0,0,255));

////            }

//            cv::putText(frame_cv, "(0,0)", cv::Point(cv_robot_origin.x+6, cv_robot_origin.y-6), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
//            cv::circle(frame_cv, cv_robot_origin, 3, cv::Scalar(0,0,255));

//            cv::putText(frame_cv, "(0,0)", cv::Point(cv_robot_origin.x+6, cv_robot_origin.y-6), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
//            cv::circle(frame_cv, cv_robot_origin, 3, cv::Scalar(0,0,255));
//            cv::threshold(frame_gray(internal), bin, 100, 255, cv::THRESH_OTSU+cv::THRESH_BINARY);
//            cv::morphologyEx(bin, eroded, cv::MORPH_OPEN, kernel_2);
//            cv::dilate(eroded, dilated, kernel_2);
//            cv::findContours(dilated, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
//            cv::namedWindow("b", cv::WINDOW_NORMAL);
//            cv::imshow("b", dilated);

//            if(!contours.empty())
//            {
//                for(std::vector<cv::Point>contour : contours)
//                {
//                    double area = cv::contourArea(contour);
//                    if(area > 1000)
//                    {
//                        cv::approxPolyDP(contour, corners, 0.05*cv::arcLength(contour,true), true);
//                        shape_1 = cv::boundingRect(corners);
//                        //cv::rectangle(frame_cv, shape, cv::Scalar(255,255,0), 1);
//                        start = cv::Point(shape_1.br().x+(frame_cv.cols/2)-400, shape_1.br().y+(frame_cv.rows/2)-200);
//                        emit pcb_found();
//                    }
//                }
//                cv::circle(frame_cv, start, 3, cv::Scalar(255,255,0),1);
//                cv::putText(frame_cv, "Start", start, cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);

//                if(mark_scanh && !is_height_measured)
//                {
//                    if((scan_height_pos.x < start.x && scan_height_pos.y < start.y) && (scan_height_pos.x > start.x-shape_1.width && scan_height_pos.y > start.y-shape_1.height))
//                    {
//                        cv::circle(frame_cv, scan_height_pos, 3, cv::Scalar(0,0,153));
//                        cv::putText(frame_cv, "Point for height measurement", cv::Point(scan_height_pos.x, scan_height_pos.y-5), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
//                        emit send_scanheight_point(scan_height_pos.x, scan_height_pos.y);
//                    }
//                    else
//                    {
//                        mark_scanh = false;
//                        emit height_scan_point_error();
//                    }
//                }
//            }
//            emit positions(zoomed_origin, cv_robot_origin.x, cv_robot_origin.y, start.x, start.y, shape_1.width, shape_1.height);
//        }
//        else if(zoomed_origin)
//        {
//            cv::cvtColor(frame_cv, frame_gray, cv::COLOR_BGR2GRAY);

//            std::vector<std::vector<cv::Point>> contours;
//            std::vector<cv::Vec4i> hierarchy;
//            cv::Mat bin;
//            cv::Mat eroded_open;
//            cv::Mat eroded;


//            cv::threshold(frame_gray, bin, 0, 255, cv::THRESH_OTSU+cv::THRESH_BINARY);

//            cv::Mat kernel = cv::Mat(2, 2, CV_8U, cv::Scalar(1));

//            cv::morphologyEx(bin, eroded, cv::MORPH_OPEN, kernel);

//            cv::findContours(eroded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

//            cv::namedWindow("a", cv::WINDOW_NORMAL);
//            cv::imshow("a",eroded);

//            cv::Rect shape;
//            cv::Point2f points[4];
//            std::vector<cv::Point2f> corners;

//            if(!contours.empty())
//            {
//                std::vector<std::vector<cv::Point>> hull( contours.size() );
//                for( size_t i = 0; i < contours.size(); i++ )
//                {
//                    convexHull( contours[i], hull[i] );
//                }

//                for(std::vector<cv::Point>contour : hull)
//                {
//                    double area = cv::contourArea(contour);
//                    if(area < desired_area+(desired_area/4) && area > desired_area-(desired_area/4))
//                    {
//                        cv::approxPolyDP(contour, corners, 0.05*cv::arcLength(contour,true), true);
//                        if(corners.size() == 4)
//                        {
//                            shape = cv::boundingRect(corners);
//                            pcb_origin = shape.br();
//                            cv::rectangle(frame_cv, shape, cv::Scalar(255,255,0), 1);
//                            cv::circle(frame_cv, cv::Point(pcb_origin.x, pcb_origin.y), 5, cv::Scalar(255,255,0),2);
//                            cv::putText(frame_cv, "Corner", cv::Point(pcb_origin.x, pcb_origin.y-5), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
//                            emit positions(1, pcb_origin.x, pcb_origin.y, 0, 0, 0, 0);
//                        }
//                    }
//                }
//            }
//        }
//        else
//        {
//            mark_scanh = false;
//            is_height_measured = false;
//            scan_height_pos.x = scan_height_pos.y = -1;
//        }
//        QImage frame_qt = MatToQImage(frame_cv);
//        emit readyImg(frame_qt);
//    }

//}

//void VideoThread::start()
//{
//    cv_camera = new cv::VideoCapture;

//    if(cv_camera->open(0, cv::CAP_ANY))
//    {
//        emit cameraOpened();
//        cv_camera->set(cv::CAP_PROP_FRAME_WIDTH, resolution_max_width);
//        cv_camera->set(cv::CAP_PROP_FRAME_HEIGHT, resolution_max_height);
//        cv_camera->set(cv::CAP_PROP_CONTRAST, -90);
//        cv_camera->set(cv::CAP_PROP_AUTOFOCUS, 0);
//        cv_camera->set(cv::CAP_PROP_EXPOSURE, -3);
//        cv_camera->set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);
//        cv_camera->set(cv::CAP_PROP_SATURATION, 20);
//        cv_camera->set(cv::CAP_PROP_BRIGHTNESS, 3);
//        timer = new QTimer;
//        connect(timer, &QTimer::timeout, this, &VideoThread::process);
//        timer->start(1);
//    }
//    else
//    {
//        emit error("No device connected!");
//        emit finished();
//    }
//}

//void VideoThread::start_detection(bool dct)
//{
//    detect = dct;
//}

//void VideoThread::scan_origin_detect(bool b)
//{
//    zoomed_origin = b;
//}

//void VideoThread::refocus(int fcs)
//{
//    cv_camera->set(cv::CAP_PROP_FOCUS, fcs);
//}

//void VideoThread::recontrast(int cv)
//{
//    cv_camera->set(cv::CAP_PROP_CONTRAST, cv);
//}

//void VideoThread::rebrightness(int cv)
//{
//    cv_camera->set(cv::CAP_PROP_BRIGHTNESS, cv);
//}

//void VideoThread::receive_area(qint64 area)
//{
//    desired_area = area;
//    qDebug() << "Area: " << area;
//}

//void VideoThread::mark_scanheight(int x, int y)
//{
//    scan_height_pos = cv::Point(x,y);
//    mark_scanh = true;
//    qDebug() << "WTF";
//}

//void VideoThread::height_measurement_done()
//{
//    is_height_measured = true;
//}

//QImage VideoThread::MatToQImage(const cv::Mat& mat)
//{
//    // 8-bits unsigned, NO. OF CHANNELS=1
//    if(mat.type()==CV_8UC1)
//    {
//        // Set the color table (used to translate colour indexes to qRgb values)
//        QVector<QRgb> colorTable;
//        for (int i=0; i<256; i++)
//            colorTable.push_back(qRgb(i,i,i));
//        // Copy input Mat
//        const uchar *qImageBuffer = (const uchar*)mat.data;
//        // Create QImage with same dimensions as input Mat
//        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
//        img.setColorTable(colorTable);
//        return img;
//    }
//    // 8-bits unsigned, NO. OF CHANNELS=3
//    if(mat.type()==CV_8UC3)
//    {
//        // Copy input Mat
//        const uchar *qImageBuffer = (const uchar*)mat.data;
//        // Create QImage with same dimensions as input Mat
//        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
//        return img.rgbSwapped();
//    }
//    return QImage();
//}

//std::vector<cv::Point> VideoThread::contourConvexHull(std::vector<cv::Point> contours)
//{
//    std::vector<cv::Point> result;
//    std::vector<cv::Point> pts;

//    for(size_t i=0; i<contours.size(); i++)
//            pts.push_back(contours[i]);
//    cv::convexHull(pts,result);
//    return result;
//}
