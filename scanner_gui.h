#ifndef SCANNER_GUI_H
#define SCANNER_GUI_H

#include <QScopedPointer>
#include <QMainWindow>
#include <QLabel>
#include <QTcpSocket>
#include <QCloseEvent>
#include <QVector>
#include <memory>
#include <QProgressDialog>
#include <QtSerialPort>

#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>

#include "videothread.h"
#include "mainsettings.h"

#include "Scan_process/Scan_wizard/scanwizard.h"
#include "Scan_process/PCB_size/dut_size.h"
#include "Scan_process/Plotting/nf_plot.h"
#include "Scan_process/Plotting/nf_plot_static.h"
#include "Scan_process/Plotting/qcustomplot.h"
#include "Scan_process/Event_log/event_log.h"
#include "Scan_process/Tool_add/tool_add.h"
#include "Scan_process/Instrument_settings/scan_settings.h"

#include "Serial/powermgm.h"

#include "TCP_IP/Robot/robot.h"
#include "TCP_IP/RS_Instr/rs_instruments.h"
#include "TCP_IP/RS_Instr/rs_sa.h"
#include "TCP_IP/RS_Instr/rs_vna.h"
#include "TCP_IP/RS_Instr/rs_instr_detector_thread.h"


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
        void insthread_stop(void);

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
        void on_rotate_probe_button_clicked();
        void on_stepsize_xy_valueChanged(double arg1);
        void on_stepsize_z_valueChanged(double arg1);
        void disp_robot_msg(QString);

        //Camera, Take and process the image
        void cv_getframe(QImage);
        void cameraError(QString);
        void cameraConnected();
        void on_actionReset_Camera_triggered();
        void throw_height_meas_error();


        //Measurement instruments slots
        void SA_online(bool);
        void VNA_online(bool);
        void on_datasave_test_clicked();

        //Tool add function
        void on_AddTool_clicked();
        void on_Tool_Tab_Closed(QVector<Tool*>);
        void on_Probe_dropdown_currentTextChanged(const QString &arg1);
        void tools_init();

        void on_actionSettings_triggered();      
        void on_actionNear_field_Plot_triggered();


        void restart_instr_detector();
        void init_progress_perform();
        void on_actionExit_2_triggered();

        void on_actionPower_management_triggered();

private:
        //Widgets
        Ui::scanner_gui *ui{};

        //Scan process
        ScanWizard *wizard{};
        Scan_interface *scan_inter{};

        //RS measurement instruments
        RS_Instruments *ins{};
        bool wo_vna{false};
        bool wo_sa{false};
        bool ins_created{false};

        //RS instrument detector thread
        Instrument_Thread *insthread{};
        void RS_instr_detector_thread_init();
        bool sa_connected_bool{false};
        bool vna_connected_bool{false};

        //Ip addresses
        const QString sa_ip_address{"192.168.11.4"};
        const QString vna_ip_address{"192.168.11.6"};

        //Init progress dialog
        QProgressDialog *init_progress{};
        std::unique_ptr<QTimer> timer{};
        int steps{};

        //OpenCV and image processing
        QImage lastImage{};
        VideoThread* videothread{};
        void video_thread_init();

        //Robot functions
        Robot robot{};
        void robot_init();

        //Plotting widget
        NF_plot *nearfield_plot{};
        NF_plot_static *static_nearfield_plot{};
        bool first_cropped{true};

        //Add tool
        QVector<Tool*> Tools{};

        //Serial port for power/LEDs management
        QSerialPort *arduino{};

        //Minor variables
        QTimer *timer2{};
        bool scan_wizard_started{false};

        //PowerMGM
        PowerMGM *powMGM{};
};

#endif // SCANNER_GUI_H
