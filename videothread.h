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

    //Camera attributes
    void refocus(int fcs)       { cv_camera->set(cv::CAP_PROP_FOCUS, fcs);     };
    void recontrast(int cv)     { cv_camera->set(cv::CAP_PROP_CONTRAST, cv);   };
    void rebrightness(int bv)   { cv_camera->set(cv::CAP_PROP_BRIGHTNESS, bv); };


    //For signals from liveStream selected by a user
    void receive_selected_pcb(int x, int y, int w, int h)        { selected_pcb = cv::Rect(x, y, w, h); disp_selected_pcb = true;      };
    void receive_selected_scan_height_point(int x, int y)        { scan_height_point = cv::Point(x, y); disp_scan_height_point = true; };
    void receive_selected_pcb_corner_point(int x, int y)         { pcb_corner_point = cv::Point(x, y);  disp_pcb_corner_point = true;  };
    void receive_selected_pcb_ref_point(int x, int y)            { pcb_ref_point = cv::Point(x, y);     disp_pcb_ref_point = true;     };

    void display_pcb_outline(bool b)        { disp_selected_pcb = b;      };
    void display_scan_height_point(bool b)  { disp_scan_height_point = b; };
    void display_pcb_corner_point(bool b)   { disp_pcb_corner_point = b;  };
    void display_pcb_ref_point(bool b)      { disp_pcb_ref_point = b;     };



    void receive_area(qint64);
    void height_measurement_done();
    void get_ref00_offsets(double, double);

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

    cv::Rect selected_pcb{};
    bool disp_selected_pcb{false};

    cv::Point scan_height_point{};
    bool disp_scan_height_point{false};

    cv::Point pcb_corner_point{};
    bool disp_pcb_corner_point{false};

    cv::Point pcb_ref_point{};
    bool disp_pcb_ref_point{false};

    cv::Point pcb_origin;
    cv::Point cv_robot_origin_fixed{640,480};

    bool detect = false;
    bool zoomed_origin = false;
    bool is_height_measured = false;

    bool mark_scanh = false;

    long int desired_area = 1e9;

    double ref00_x_off = 0, ref99_y_off = 0;
    QImage MatToQImage(const cv::Mat& mat);
    const uint16_t resolution_max_width = 4208;
    const uint16_t resolution_max_height = 3120;
};

#endif
