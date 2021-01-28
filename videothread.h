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
    void start_detection(bool);
    void scan_origin_detect(bool);
    void refocus(int);
    void recontrast(int);
    void rebrightness(int);
    void receive_area(qint64);
    void mark_scanheight(int ,int);
    void height_measurement_done();
    void get_ref00_offsets(double, double);
    void kill_window();

signals:
    void finished();
    void error(QString err);
    void cameraOpened();
    void readyImg(QImage);
    void positions(bool, int, int, int, int, int, int);
    void pcb_found();
    void corner_found();
    void send_scanheight_point(int, int);
    void height_scan_point_error();

private:
    cv::VideoCapture * cv_camera;
    QTimer * timer;
    QImage MatToQImage(const cv::Mat& mat);
    cv::Point pcb_origin;
    cv::Point cv_robot_origin_fixed = cv::Point(82,160);
    bool detect = false;
    bool zoomed_origin = false;
    bool is_height_measured = false;
    cv::Point scan_height_pos;
    bool mark_scanh = false;

    long int desired_area = 1e9;

    double ref00_x_off = 0, ref99_y_off = 0;

    const uint16_t resolution_max_width = 4208;
    const uint16_t resolution_max_height = 3120;
};

#endif
