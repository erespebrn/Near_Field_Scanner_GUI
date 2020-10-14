#ifndef SCANNER_GUI_H
#define SCANNER_GUI_H

#include <QScopedPointer>
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRubberBand>
#include <QTcpSocket>
#include <QPainter>
#include <QWizard>
#include <QCloseEvent>
#include "videothread.h"
#include "instrument_thread.h"
#include "scanwizard.h"

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
    void reset();
    ~scanner_gui();

signals:
    void insthread_stop();
    void send_coord_to_wizard(QPoint, QRect);
    void cropped_image_coord();

private slots:

    //Scan start stop
    void on_Start_scan_button_clicked();
    void stop_scan_button_clicked();
    void closeEvent(QCloseEvent *event) override;

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
    void measure_height_clicked();

    //Take and process the image
    void Take_img_button_clicked();
    void displayCapturedImage();
    void processCapturedImage(const QImage &img);
    void displayCroppedImage(QRect& rect);

    //Camera settings
    void resetCamera_button_clicked();

    //Camera recording settings

    void displayViewfinder();
    void on_scan_settings_button_clicked();

    //void on_actionSettings_triggered();

    void SA_online(bool);
    void VNA_online(bool);

    void cv_getframe(QImage);
    void cv_getcoord(bool, int, int, int, int, int, int);
    void cameraError(QString);
    void cameraConnected();
    void on_robot_connect_button_clicked();

    void read_robot_msg();
    void wizard_robot_to_origin(bool);

    void on_actionReset_Camera_triggered();

    void on_datasave_test_clicked();
    void wizard_mark_background(int);
    void scan_control(bool);
    void sa_dataread();
    void get_sweep_points(qint32);

private:
    Ui::scanner_gui *ui;
    ScanWizard * wizard;

    //TCP sockets
    QTcpSocket *_socket_sa;
    QTcpSocket _socket_vna;
    QTcpSocket *_socket_robot;

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

    uint16_t camera_distance_2 = 230;

    const uint16_t resolution_max_width = 4208;
    const uint16_t resolution_max_height = 3120;

    //OpenCV
    cv::Point cv_robot_origin;
    cv::Mat cv_lastImage;
    QImage lastImage;
    QRect croppedOrigin;
    VideoThread* videothread;
    void video_thread_init();
    void instrument_thread_init();
    void robot_init();
    void send_robot_coordinates(bool);
    void set_scan_step_sizes();
    void start_scan();
    void stop_scan();

    QPoint origin;
    QPoint pcb_corner;
    QRect pcb_size;
    QPoint scan_pcb_corner;
    QPoint scan_area_corner;
    QRect scan_area_size;

    QColor laststyle;
    bool picture_taken = false;

    qint32 sweep_points;
};

#endif // SCANNER_GUI_H
