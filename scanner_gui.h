#ifndef SCANNER_GUI_H
#define SCANNER_GUI_H

#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QScopedPointer>
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRubberBand>
#include <QTcpSocket>
#include <QPainter>

#include "videothread.h"
#include "instrument_thread.h"

#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class scanner_gui; }
QT_END_NAMESPACE

class scanner_gui : public QMainWindow
{
    Q_OBJECT

public:
    scanner_gui();
    ~scanner_gui();

signals:
    void insthread_stop();
private slots:

    //Scan start stop
    void on_Start_scan_button_clicked();
    void on_stop_scan_button_clicked();

    //Robot control buttons and settings
    void on_Y_plus_button_pressed();
    void on_Y_minus_button_pressed();
    void on_X_plus_button_pressed();
    void on_X_minus_button_pressed();
    void on_Z_plus_pressed();
    void on_Z_minus_pressed();
    void on_home_button_clicked();
    void on_stepsize_x_valueChanged(double arg1);
    void on_stepsize_y_valueChanged(double arg1);
    void on_scan_height_valueChanged(double arg1);
    void on_measure_height_clicked();

    //Take and process the image
    void on_Take_img_button_clicked();
    void displayCapturedImage();
    void processCapturedImage(const QImage &img);
    void displayCroppedImage(QRect& rect);

    //Camera settings
    void on_resetCamera_button_clicked();

    //Camera recording settings

    void displayViewfinder();
    void on_scan_settings_button_clicked();

    //void on_actionSettings_triggered();

    void on_camera_connect_button_clicked();

    void SA_online(bool);
    void VNA_online(bool);

    void cv_getframe(QImage, int, int);
    void cameraError(QString);
    void cameraConnected();
    void on_robot_connect_button_clicked();

    void read_robot_msg();

private:
    Ui::scanner_gui *ui;

    //TCP sockets
    QTcpSocket _socket_sa;
    QTcpSocket _socket_vna;

    QTcpSocket _socket_robot;

    // IP addresses
    const QString sa_ip_address = "192.168.11.4";
    const QString vna_ip_address = "192.168.11.6";
    const QString robot_ip_address = "192.168.11.2";

    bool sa_connected_bool = false;
    bool vna_connected_bool = false;

    // Camera image sensor dimensions
    const float sensor_width = 4.54;
    const float sensor_height = 3.42;
    const float focal_lenght = 3.81;
    uint16_t camera_distance = 890;

    const uint16_t resolution_max_width = 4208;
    const uint16_t resolution_max_height = 3120;

    //OpenCV
    cv::Point cv_robot_origin;
    cv::Mat cv_lastImage;
    QImage lastImage;
    QRect croppedOrigin;

    void video_thread_init();
    void instrument_thread_init();
    void robot_init();
    void send_robot_coordinates();

    uint16_t origin_x;
    uint16_t origin_y;

    uint16_t scan_size_x;
    uint16_t scan_size_y;

    bool picture_taken = false;
};

#endif // SCANNER_GUI_H
