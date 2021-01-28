#ifndef SCANNER_GUI_H
#define SCANNER_GUI_H

#include <QScopedPointer>
#include <QMainWindow>
#include <QLabel>
#include <QTcpSocket>
#include <QCloseEvent>
#include <QVector>

#include "videothread.h"
#include "instrument_thread.h"
#include "scanwizard.h"
#include "tool.h"
#include "robot.h"
#include "dut_size.h"
#include "mainsettings.h"

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
        void init();
        ~scanner_gui();

    signals:
        void insthread_stop();
        void send_coord_to_wizard(QPoint, QRect);
        void cropped_image_coord();
        void height_measured();

        void send_area_to_videothread(qint64);
        void stop_displaying_point();
        void allow_emit_pos(bool);
        void instruments_created();

    private slots:

        //Scan start stop
        void on_Start_scan_button_clicked();
        //void stop_scan_button_clicked();
        void closeEvent(QCloseEvent *event) override;

        //Robot control buttons and settings
        void on_robot_connect_button_clicked();
        void on_Y_plus_button_pressed();
        void on_Y_minus_button_pressed();
        void on_X_plus_button_pressed();
        void on_X_minus_button_pressed();
        void on_Z_plus_pressed();
        void on_Z_minus_pressed();
        void on_home_button_clicked();
        void on_stepsize_xy_valueChanged(double arg1);
        void on_stepsize_z_valueChanged(double arg1);
        void on_scan_height_valueChanged(double arg1);
        void disp_robot_msg(QString);

        //Camera, Take and process the image
        void take_img();
        void displayCapturedImage();
        void processCapturedImage(const QImage &img);
        void displayCroppedImage(QRect& rect);
        void save_cropped_img();
        void reset_camera();
        void cv_getframe(QImage);
        void cameraError(QString);
        void cameraConnected();
        void on_actionReset_Camera_triggered();
        void throw_height_meas_error();

        //Scan settings
        void on_scan_settings_button_clicked();

        //Measurement instruments slots
        void SA_online(bool);
        void VNA_online(bool);
        void on_datasave_test_clicked();

        //Wizard slots
        void wizard_mark_background(int);
        void current_scan_stopped();
        void create_datapath(QString);

        //Tool add function
        void on_AddTool_clicked();
        void on_Tool_Tab_Closed(QVector<Tool*>);
        void on_Probe_dropdown_currentTextChanged(const QString &arg1);
        void tools_init();

        void on_actionSettings_triggered();
        void on_rotate_probe_button_clicked();

private:
        //Widgets
        Ui::scanner_gui *ui;
        ScanWizard * wizard;

        //RS measurement instruments
        RS_Instruments *ins;
        bool wo_vna = false;
        bool wo_sa = false;
        bool ins_created = false;

        //RS instrument detector thread
        Instrument_Thread * insthread;
        bool sa_connected_bool = false;
        bool vna_connected_bool = false;

        //Ip addresses
        const QString sa_ip_address = "192.168.11.4";
        const QString vna_ip_address = "192.168.11.6";

        //PCB and scan area objects
        DUT_size *pcb;
        DUT_size *scan_area;
        QString dut_name;

        //OpenCV and image processing
        QImage lastImage;
        QImage croppedImg;
        QRect croppedOrigin;
        VideoThread* videothread;
        void video_thread_init();
        void instrument_thread_init();

        //Robot functions
        Robot *robot;
        void robot_init();
        void set_scan_step_sizes();
        void start_scan();

        //Data storage variables and functions
        QString datapath = "C:/Users/Near-field scanner/Documents/Near_Field_Scanner_GUI/datastorage/scan_data/";
        QString current_scan_datapath;

        //Add tool
        QVector<Tool*> Tools;

        //Minor variables
        QColor laststyle;
        uint16_t step_size = 0;
        QTimer * timer2;
        bool scan_wizard_started = false;
};

#endif // SCANNER_GUI_H
