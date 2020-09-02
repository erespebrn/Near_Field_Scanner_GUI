#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QObject>
#include <QImage>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/photo/photo.hpp>

class VideoThread : public QObject {
    Q_OBJECT

public:
    VideoThread();
    ~VideoThread();

public slots:
    void process();
    void start();

signals:
    void finished();
    void error(QString err);
    void cameraOpened();
    void readyImg(QImage, int, int);

private:
    cv::Mat mat;
    cv::VideoCapture * cv_camera;
    QTimer * timer;
    QImage MatToQImage(const cv::Mat& mat);
    const uint16_t resolution_max_width = 4208;
    const uint16_t resolution_max_height = 3120;
};

#endif