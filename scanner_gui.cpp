#include "scanner_gui.h"
#include "ui_scanner_gui.h"

#include <cstdio>
#include <fstream>
#include <cmath>
#include <QDebug>
#include <ctype.h>
#include <QTimer>
#include <QMessageBox>
#include <QtWidgets>
#include <QLabel>
#include <vector>
#include <QPushButton>
#include <QHostAddress>
#include <QApplication>
#include <QProcess>
#include <new>

#include "Scan_process/Plotting/nf_plot_vna.h"
#include "Scan_process/Plotting/nf_plot_sa.h"

//Constructor destructor
scanner_gui::scanner_gui() : ui(new Ui::scanner_gui)
{
    ui->setupUi(this);

    //Init components
    this->init();

    //Init progress bar
    init_progress = new QProgressDialog("Intialization..", "Cancel", 0, 100, this);
    init_progress->setWindowModality(Qt::WindowModal);
    init_progress->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    init_progress->setCancelButton(0);
    timer = std::unique_ptr<QTimer>(new QTimer(this));
    connect(timer.get(), &QTimer::timeout, this, &scanner_gui::init_progress_perform);
    timer->start(40);

    //Change the cursor
    QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));

    //CIE logo
    QImage *logo = new QImage(":/img/images/cie_logo_big.png");
    QPainter p;
    p.begin(logo);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(logo->rect(), QColor(0, 0, 0, 100));
    p.end();
    *logo = logo->scaled(ui->cie_logo_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->cie_logo_label->setPixmap(QPixmap::fromImage(*logo));
    delete logo;

    //Pushbuttons/dial init state
    ui->datasave_test->setVisible(false);
    ui->camera_focus_dial->setEnabled(false);

    //Serial init for the power/LED management
    arduino = new QSerialPort;
    arduino->setPortName("COM4");
    arduino->setBaudRate(QSerialPort::Baud9600);
    arduino->setDataBits(QSerialPort::Data8);
    arduino->setParity(QSerialPort::NoParity);
    arduino->setStopBits(QSerialPort::OneStop);
    arduino->setFlowControl(QSerialPort::NoFlowControl);
    arduino->open(QSerialPort::ReadWrite);

    //Power MGM widget
    powMGM = new PowerMGM(nullptr, arduino);
    powMGM->setWindowModality(Qt::WindowModal);
    powMGM->setWindowTitle("Power Management");
}

void scanner_gui::init_progress_perform()
{
    init_progress->setValue(steps);
    steps++;
    if (steps>init_progress->maximum())
        timer->stop();
}

scanner_gui::~scanner_gui()
{
    emit videothread->finished();
    emit insthread->finished();
    delete insthread;
    delete arduino;
    delete ui;
    delete powMGM;
}

//Init functions
void scanner_gui::init()
{
    this->robot_init();
    this->video_thread_init();
    this->RS_instr_detector_thread_init();

    // Delete last scan settings file
    QFile file(QCoreApplication::applicationDirPath() + "/settings/scansettings.ini");
    if(file.exists())
        file.remove();

    // Load robot tool data
    this->tools_init();

}

void scanner_gui::robot_init()
{
    connect(&robot, &Robot::robot_msg_to_terminal, this, &scanner_gui::disp_robot_msg);

    if(robot.init()){
        ui->robot_connect_button->setEnabled(false);
        ui->robot_connect_button->setText("Connected");
        ui->robotTerminal->setText("");
        ui->robotTerminal->setText("Connected to Kawasaki Robot Controller");
        ui->robotManualControl_frame->setEnabled(true);
    }else{
        ui->robotManualControl_frame->setEnabled(false);
    }
}

void scanner_gui::video_thread_init()
{
    QThread* thread1 = new QThread;
    videothread = new VideoThread;
    videothread->moveToThread(thread1);

    connect(thread1,     SIGNAL(started()),        videothread, SLOT(start()));
    connect(thread1,     SIGNAL(finished()),       thread1,     SLOT(deleteLater()));
    connect(videothread, SIGNAL(finished()),       thread1,     SLOT(quit()));
    connect(videothread, SIGNAL(finished()),       videothread, SLOT(deleteLater()));

    connect(videothread, SIGNAL(readyImg(QImage)),          this, SLOT(cv_getframe(QImage)));
    connect(videothread, SIGNAL(error(QString)),            this, SLOT(cameraError(QString)));
    connect(videothread, SIGNAL(cameraOpened()),            this, SLOT(cameraConnected()));
    connect(videothread, SIGNAL(height_scan_point_error()), this, SLOT(throw_height_meas_error()));

    connect(ui->camera_focus_dial,      SIGNAL(valueChanged(int)),  videothread, SLOT(refocus(int)));
    connect(ui->camera_contrast_dial,   SIGNAL(valueChanged(int)),  videothread, SLOT(recontrast(int)));
    connect(ui->camera_brightness_dial, SIGNAL(valueChanged(int)),  videothread, SLOT(rebrightness(int)));

    connect(&robot, SIGNAL(stop_displaying_point()), videothread, SLOT(height_measurement_done()));
    thread1->start();
}

void scanner_gui::RS_instr_detector_thread_init()
{
    QThread * thread2 = new QThread;
    insthread = new Instrument_Thread(sa_ip_address, vna_ip_address);
    insthread->moveToThread(thread2);
    connect(thread2, SIGNAL(started()), insthread, SLOT(start()));
    connect(thread2, SIGNAL(finished()), thread2, SLOT(deleteLater()));
    connect(insthread, SIGNAL(finished()), thread2, SLOT(quit()));
    connect(insthread, SIGNAL(finished()), insthread, SLOT(deleteLater()));
    connect(insthread, SIGNAL(VNA_connected(bool)), this, SLOT(VNA_online(bool)));
    connect(insthread, SIGNAL(SA_connected(bool)), this, SLOT(SA_online(bool)));
    connect(this, SIGNAL(insthread_stop()), insthread, SLOT(shutdown()));
    thread2->start();
}

void scanner_gui::on_robot_connect_button_clicked()
{
    this->robot_init();
}


void scanner_gui::cameraConnected()
{
    ui->actionReset_Camera->setEnabled(false);
    ui->actionReset_Camera->setText("Connected");
}

void scanner_gui::cameraError(QString error)
{
    ui->actionReset_Camera->setEnabled(true);
    ui->actionReset_Camera->setText("Connect camera");
    ui->liveStream->setText("Camera error. Try to reconect!");
    QMessageBox::critical(this, "Camera error", error);
}

void scanner_gui::cv_getframe(QImage frame)
{
    QImage scaledframe_cv = frame.scaledToWidth(ui->liveStream->width(), Qt::SmoothTransformation);
    lastImage = scaledframe_cv;
    ui->liveStream->setPixmap(QPixmap::fromImage(scaledframe_cv));
}

void scanner_gui::on_actionReset_Camera_triggered()
{
    this->video_thread_init();
}

void scanner_gui::throw_height_meas_error()
{
    QMessageBox::warning(this, "Point outside PCB", "Selected point is outside the PCB.\nSelect a new point!");
}

//Scan wizard functions
void scanner_gui::on_Start_scan_button_clicked()
{
    if(robot.state() == QAbstractSocket::ConnectedState){
        //The Scan wizard
        wizard = new ScanWizard(this, &robot, videothread, this->ui);
        wizard->setAttribute(Qt::WA_DeleteOnClose);

        wizard->setWindowFlag(Qt::WindowStaysOnTopHint);
        wizard->setWindowFlags(wizard->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        wizard->setFixedSize(450,350);
        wizard->setWindowTitle("Scan Wizard");
        wizard->move(1430,550);
        wizard->setStyleSheet("background-color: rgba(204,204,204,1)");
        wizard->show();

        scan_wizard_started = true;
    }else{
        QMessageBox::critical(this, "Critial error!", "Scan cannot be performed when the robot is offline. Turn on the robot, connect using the button on the right toolbar and try again!");
    }
}

//Scan control functions
void scanner_gui::restart_instr_detector()
{
    this->RS_instr_detector_thread_init();
}


//Measurement instruments functions
void scanner_gui::SA_online(bool state)
{
    sa_connected_bool = state;
    if(state)
        ui->SA_indicator->setPixmap(QPixmap(":/img/images/led_on.png"));
    else
        ui->SA_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));

    if(scan_wizard_started)
        wizard->sa_connected(state);
}

void scanner_gui::VNA_online(bool state)
{
    vna_connected_bool = state;
    if(state)
        ui->VNA_indicator->setPixmap(QPixmap(":/img/images/led_on.png"));
    else
        ui->VNA_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));

    if(scan_wizard_started)
        wizard->vna_connected(state);
}

void scanner_gui::on_datasave_test_clicked()
{
    scan_settings scan_settings(nullptr, new RS_sa);
    scan_settings.setStyleSheet("");
    scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    scan_settings.setModal(true);
    scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
    scan_settings.exec();
}

//Robot control
void scanner_gui::disp_robot_msg(QString msg)
{
    ui->robotTerminal->setText(msg);
}

void scanner_gui::on_stepsize_xy_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
}

void scanner_gui::on_stepsize_z_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
}

void scanner_gui::on_Y_plus_button_pressed()
{
    robot.Y_shift(-ui->stepsize_xy->value());
}

void scanner_gui::on_Y_minus_button_pressed()
{
    robot.Y_shift(ui->stepsize_xy->value());
}

void scanner_gui::on_X_plus_button_pressed()
{
    robot.X_shift(-ui->stepsize_xy->value());
}

void scanner_gui::on_X_minus_button_pressed()
{
    robot.X_shift(ui->stepsize_xy->value());
}

void scanner_gui::on_Z_plus_pressed()
{
    robot.Z_shift(ui->stepsize_z->value());
    robot.ask_for_camera_height();
}

void scanner_gui::on_Z_minus_pressed()
{
    robot.Z_shift(-ui->stepsize_z->value());
    robot.ask_for_camera_height();
}

void scanner_gui::on_home_button_clicked()
{
    robot.goto_home_pos();
}

void scanner_gui::on_rotate_probe_button_clicked()
{
    robot.probe_rotate();
}

//Main settings
void scanner_gui::on_actionSettings_triggered()
{
    MainSettings mainsettings(nullptr, &robot);
    connect(&mainsettings, SIGNAL(send_ref00_offsets(double, double)), videothread, SLOT(get_ref00_offsets(double, double)));
    mainsettings.setWindowFlags(mainsettings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    mainsettings.setModal(true);
    mainsettings.setFixedSize(mainsettings.width(),mainsettings.height());
    mainsettings.exec();
}

void scanner_gui::on_actionNear_field_Plot_triggered()
{
    //*** Plotting widget
    NF_plot_static static_nearfield_plot;
}

//Tool add functions
void scanner_gui::on_Probe_dropdown_currentTextChanged(const QString &arg1)
{
    QString probe = arg1;
    int t_x, t_y, t_z;

    ///Find the probe in the vector
    for (int i = 0; i < Tools.length(); i++) {
        if (Tools[i]->tool_name == probe){
            t_x = Tools[i]->tool_x;
            t_y = Tools[i]->tool_y;
            t_z = Tools[i]->tool_z;
        }
    }
    robot.tool_change(t_x, t_y, t_z);
}

void scanner_gui::tools_init()
{
    QFile inputFile(QCoreApplication::applicationDirPath() + "/settings/tooldata.txt");
    if (inputFile.open(QIODevice::ReadOnly)){
        QTextStream in(&inputFile);
        while (!in.atEnd()){
            QString line = in.readLine();
            QStringList fields = line.split(";");
            Tool* T = new Tool(fields[0],fields[1].toFloat(),fields[2].toFloat(),fields[3].toFloat());
            Tools.append(T);
        }
        inputFile.close();
    }
    ///Fill in the "Probes" dropdown
    for (int i = 0; i < Tools.length(); i++) {
        ui->Probe_dropdown->addItem(Tools[i]->tool_name);
    }

}

void scanner_gui::on_AddTool_clicked()
{
    tool_add* tool_add_ = new tool_add(this);
    connect(tool_add_, SIGNAL(tool_tab_closed(QVector<Tool*>)), this, SLOT(on_Tool_Tab_Closed(QVector<Tool*>)));
    tool_add_->setWindowFlags(tool_add_->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    tool_add_->setModal(true);
    tool_add_->setFixedSize(tool_add_->width(),tool_add_->height());
    tool_add_->exec();
}

void scanner_gui::on_Tool_Tab_Closed(QVector<Tool*> rTools)
{
    ui->Probe_dropdown->clear();
    Tools = rTools;
    for (int i = 0; i < Tools.length(); i++) {
        ui->Probe_dropdown->addItem(Tools[i]->tool_name);
    }
}


//Close event
void scanner_gui::closeEvent (QCloseEvent *event)
{
    Q_UNUSED(event);
}

void scanner_gui::on_actionExit_2_triggered()
{
    QCoreApplication::quit();
}

void scanner_gui::on_actionPower_management_triggered()
{   
    powMGM->show();
}
