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
#include <QVector>

//#include "videothread.h"
#include "instrument_thread.h"
#include "scanwizard.h"
#include "tool.h"
#include "robot.h"
#include "dut_size.h"

//#include <opencv2/core/core.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/videoio/videoio.hpp>

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
        void stop_scan_button_clicked();
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
        void stop_current_scan();

        void on_Probe_dropdown_currentTextChanged(const QString &arg1);
        void tools_init();

        void robotBytesWritten(qint64);


        //Camera, Take and process the image
        void Take_img_button_clicked();
        void displayCapturedImage();
        void processCapturedImage(const QImage &img);
        void displayCroppedImage(QRect& rect);
        void resetCamera_button_clicked();
        void displayViewfinder();
        void cv_getframe(QImage);
        void cv_getcoord(bool, int, int, int, int, int, int);
        void cameraError(QString);
        void cameraConnected();
        void on_actionReset_Camera_triggered();
        void receive_scanheight_point(int, int);
        void throw_height_meas_error();

        //Scan settings
        void on_scan_settings_button_clicked();
        void get_sweep_points_amount(int);

        //Measurement instruments slots
        void SA_online(bool);
        void VNA_online(bool);
        void on_datasave_test_clicked();
        void get_trace_data(bool);
        void sa_dataread();
        void vna_dataread();
        void confirm_written_bytes(qint64);

        //Wizard slots
        void wizard_robot_to_origin(bool);
        void wizard_mark_background(int);
        void wizard_scan_control(bool);
        void send_to_takepic2_pos();
        void ask_robot_for_cam_height();
        void ask_for_cam_h();

        //Data management
        /**
         * @brief Saves the scan data for given field component\n
         * Pass 'x' or 'y' as an argument to save corresponding field data.
         */
        bool save_scan_data(char);

        void on_AddTool_clicked();
        void on_Tool_Tab_Closed(QVector<Tool*>);

private:
        //Widgets
        Ui::scanner_gui *ui;
        ScanWizard * wizard;

        //RS measurement instruments
        RS_Instruments *sa;
        RS_Instruments *vna;

        //RS instrument detector thread
        Instrument_Thread * insthread;
        bool sa_connected_bool = false;
        bool vna_connected_bool = false;

        //PCB and scan area objects
        DUT_size *pcb;
        DUT_size *scan_area;

        //OpenCV and image processing
        QImage lastImage;
        QRect croppedOrigin;
        //VideoThread* videothread;
        void video_thread_init();
        void instrument_thread_init();

        //Robot functions
        Robot *robot;
        void robot_init();
        void set_scan_step_sizes();
        void start_scan();


        //Data storage variables and functions
        QString datapath = "C:/Users/Near-field scanner/Documents/Near_Field_Scanner_GUI/datastorage/scan_data/";
        QString foldername = "SCAN_21_10_2020__15_34_11";
        QString current_scan_datapath;
        uint16_t scan_point = 0;
        QVector<Tool*> Tools;

        QTimer * timer2;

        //Minor variables
        QColor laststyle;
        bool time_for_amplitude = false;
        bool robot_first_run = true;
};

#endif // SCANNER_GUI_H
