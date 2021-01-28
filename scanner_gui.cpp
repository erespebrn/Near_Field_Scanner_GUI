
#include "scanner_gui.h"
#include "scan_settings.h"
#include "ui_scanner_gui.h"
#include "tool.h"
#include "tool_add.h"
#include <cstdio>

#include <fstream>
#include <cmath>
#include <QDebug>
#include <ctype.h>
#include <QTimer>
#include <QMessageBox>
#include <QtWidgets>
#include <QLabel>
#include <QFile>
#include <iterator>
#include <vector>
#include <QPushButton>
#include <QHostAddress>
#include <QApplication>
#include <QProcess>
#include <new>

#include "rs_instruments.h"

//Constructor destructor
scanner_gui::scanner_gui() : ui(new Ui::scanner_gui)
{
    ui->setupUi(this);
    ui->datasave_test->setVisible(false);
}

scanner_gui::~scanner_gui()
{
    delete videothread;
    delete wizard;
    delete robot;
    delete ui;
}

//Init functions
void scanner_gui::init()
{
    robot_init();
    video_thread_init();
    instrument_thread_init();

    //Mouse events signals
    connect(ui->lastImagePreviewLabel, SIGNAL(sendQrect(QRect&)), this, SLOT(displayCroppedImage(QRect&)));
    connect(ui->liveStream, SIGNAL(sendPos(int, int)), videothread, SLOT(mark_scanheight(int ,int)));

    //Signal to allow emiting the search height point
    connect(robot, SIGNAL(allow_emit_pos(bool)), ui->liveStream, SLOT(allow_emit(bool)));

    // Delete last scan settings file
    QFile file(QCoreApplication::applicationDirPath() + "/settings/scansettings.ini");
    if(file.exists())
        file.remove();

    // Minor init settings
    ui->stackedWidget->setCurrentIndex(0);

    // Load robot tool data
    tools_init();
}

void scanner_gui::robot_init()
{
    robot = new Robot;
    connect(robot, &Robot::robot_msg_to_terminal, this, &scanner_gui::disp_robot_msg);
    connect(robot, &Robot::save_cropped_image, this, &scanner_gui::save_cropped_img);
    if(robot->tcp_connect()){
        ui->robot_connect_button->setEnabled(false);
        ui->robot_connect_button->setText("Connected");
        ui->robotTerminal->setText("");
        ui->robotTerminal->setText("Connected to Kawasaki F Controller");
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

    connect(thread1, SIGNAL(started()), videothread, SLOT(start()));
    connect(thread1, SIGNAL(finished()), thread1, SLOT(deleteLater()));
    connect(videothread, SIGNAL(finished()), thread1, SLOT(quit()));
    connect(videothread, SIGNAL(finished()), videothread, SLOT(deleteLater()));
    connect(videothread, SIGNAL(readyImg(QImage)), this, SLOT(cv_getframe(QImage)));
    connect(videothread, SIGNAL(error(QString)), this, SLOT(cameraError(QString)));
    connect(videothread, SIGNAL(cameraOpened()), this, SLOT(cameraConnected()));
    connect(ui->camera_focus_dial, SIGNAL(valueChanged(int)), videothread, SLOT(refocus(int)));
    connect(ui->camera_contrast_dial, SIGNAL(valueChanged(int)), videothread, SLOT(recontrast(int)));
    connect(ui->camera_brightness_dial, SIGNAL(valueChanged(int)), videothread, SLOT(rebrightness(int)));
    connect(robot, SIGNAL(stop_displaying_point()), videothread, SLOT(height_measurement_done()));
    connect(videothread, SIGNAL(height_scan_point_error()), this, SLOT(throw_height_meas_error()));
    thread1->start();
}

void scanner_gui::instrument_thread_init()
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
    robot_init();
}


//Video and image processing functions
void scanner_gui::take_img()
{
    displayCapturedImage();
    processCapturedImage(lastImage);
}

void scanner_gui::reset_camera()
{

    ui->stackedWidget->setCurrentIndex(0);
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
    QImage scaledframe_cv = frame.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    lastImage = scaledframe_cv;
    ui->liveStream->setPixmap(QPixmap::fromImage(scaledframe_cv));
}

void scanner_gui::processCapturedImage(const QImage &img)
{
    QImage scaledImage = img.scaled(ui->lastImagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    displayCapturedImage();
}

void scanner_gui::displayCroppedImage(QRect &rect)
{
    const QPixmap* pixmap = ui->lastImagePreviewLabel->pixmap();
    QImage image( pixmap->toImage() );
    QImage cropped = image.copy(rect);
    QImage scaledImage = cropped.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    croppedImg = scaledImage;
    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
}

void scanner_gui::save_cropped_img()
{
    croppedImg.save(current_scan_datapath+"cropped_image.png", "PNG",100);
}

void scanner_gui::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void scanner_gui::on_actionReset_Camera_triggered()
{
    video_thread_init();
}

void scanner_gui::throw_height_meas_error()
{
    QMessageBox::warning(this, "Point outside PCB", "Selected point is outside the PCB.\nSelect a new point!");
}


//Scan wizard functions
void scanner_gui::on_Start_scan_button_clicked()
{
    if(robot->_socket_robot->state() == QAbstractSocket::ConnectedState){
        //Detected PCB and scan area
        pcb = new DUT_size;
        scan_area = new DUT_size;
        robot->assign_duts(pcb, scan_area);
        robot->allow_emit_area(true);
        connect(videothread, SIGNAL(send_scanheight_point(int, int)), pcb, SLOT(receive_scanheight_point(int, int)));
        connect(pcb, SIGNAL(send_area_to_videothread(qint64)), videothread, SLOT(receive_area(qint64)));
        connect(ui->lastImagePreviewLabel, SIGNAL(sendQrect(QRect&)), scan_area, SLOT(receive_cropped_area(QRect&)));
        connect(videothread, SIGNAL(positions(bool, int, int, int, int, int, int)), pcb, SLOT(cv_getcoord(bool, int, int, int, int, int, int)));

        //Load mainsettings offsets
        MainSettings *ms = new MainSettings(nullptr, robot);
        connect(ms, SIGNAL(send_ref00_offsets(double, double)), videothread, SLOT(get_ref00_offsets(double, double)));
        ms->read_previous_values();

        wizard = new ScanWizard(this, robot, pcb, scan_area);
        connect(wizard, SIGNAL(detect_pcb(bool)), videothread, SLOT(start_detection(bool)));
        connect(videothread, SIGNAL(pcb_found()), wizard, SLOT(pcb_found()));
        connect(wizard, SIGNAL(kill_cv_window()), videothread, SLOT(kill_window()));
        connect(pcb, SIGNAL(send_coord_to_wizard(QPoint, QRect)), wizard, SLOT(take_coord(QPoint, QRect)));
        connect(wizard, SIGNAL(scan_area_origin_detect(bool)), videothread, SLOT(scan_origin_detect(bool)));
        connect(wizard, SIGNAL(set_scan_settings(int)), this, SLOT(wizard_mark_background(int)));
        connect(wizard, SIGNAL(scan_stopped()), this, SLOT(current_scan_stopped()));
        connect(robot, SIGNAL(height_measured()), wizard, SLOT(height_measure_finished()));
        connect(robot, SIGNAL(scan_finished_to_wizard()), wizard, SLOT(scan_finished()));
        connect(robot, SIGNAL(scan_error()), wizard, SLOT(scan_error()));
        connect(wizard, SIGNAL(allow_emit_pos(bool)), ui->liveStream, SLOT(allow_emit(bool)));
        connect(this, SIGNAL(instruments_created()), wizard, SLOT(inst_created()));
        connect(wizard, SIGNAL(send_dut_name(QString)), this, SLOT(create_datapath(QString)));
        connect(ui->liveStream, SIGNAL(height_meas_point_selected()), wizard, SLOT(height_meas_point_selected()));

        emit on_stepsize_xy_valueChanged(ui->stepsize_xy->value());
        wizard->setWindowFlag(Qt::WindowStaysOnTopHint);
        //wizard->adjustSize();
        wizard->setWindowFlags(wizard->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        wizard->setFixedSize(400,300);
        wizard->setWindowTitle("Scan Wizard");
        wizard->move(1300,100);
        wizard->setStyleSheet("background-color: rgba(180,210,210,1)");
        wizard->show();
        scan_wizard_started = true;
    }else{
        QMessageBox::critical(this, "Critial error!", "Scan cannot be performed when the robot is offline. Turn on the robot, connect using button on the right toolbar and try again!");
    }
    wizard_mark_background(10);
}

void scanner_gui::create_datapath(QString dut)
{
    dut_name = dut;
    current_scan_datapath = datapath + "SCAN_" + dut + "__" + QDate::currentDate().toString("dd_MM_yyyy") + "__" + QTime::currentTime().toString("hh_mm_ss") + "/";
}

void scanner_gui::wizard_mark_background(int r)
{
    switch(r)
    {
        case(3):
        {
            ui->scan_height->setStyleSheet("background-color: rgb(150,220,150)");
            break;
        }
        case(5):
        {
            ui->scan_height->setStyleSheet("");
            ui->robotManualControl_frame->setStyleSheet("background-color: rgb(150,220,150)");
            break;
        }
        case(6):
        {
            ui->robotManualControl_frame->setStyleSheet("");
            laststyle = ui->scan_settings_button->palette().button().color();
            ui->scan_settings_button->setStyleSheet("background-color: rgb(150,220,150)");
            break;
        }
        case(7):
        {
            ui->scan_settings_button->setStyleSheet("");
            ui->stepsize_xy->setStyleSheet("background-color: rgb(150,220,150)");
            break;
        }
        case(8):
        {
            ui->stepsize_xy->setStyleSheet("");
            ui->stepsize_z->setStyleSheet("");
            ui->scan_height->setStyleSheet("");
            break;
        }
        case(10):
        {
            ui->robotManualControl_frame->setStyleSheet("");
            ui->scan_settings_button->setStyleSheet("");
            ui->stepsize_xy->setStyleSheet("");
            ui->stepsize_z->setStyleSheet("");
            ui->scan_height->setStyleSheet("");
            break;
        }
    }
}


//Scan control functions
void scanner_gui::current_scan_stopped()
{
    this->instrument_thread_init();
    robot->stop_scan();
    robot->goto_home_pos();
    robot->allow_emit_area(true);
    scan_wizard_started = false;
    delete pcb;
    delete scan_area;
    if(ins_created){
        ins->log_file->save_to_new_logfile();
        ins_created = false;
        delete ins;
    }

}

//Measure instruments functions
void scanner_gui::SA_online(bool state)
{
    sa_connected_bool = state;
    if(state)
        ui->SA_indicator->setPixmap(QPixmap(":/img/images/led_on.png"));
    else
        ui->SA_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));
}

void scanner_gui::VNA_online(bool state)
{
    vna_connected_bool = state;
    if(state)
        ui->VNA_indicator->setPixmap(QPixmap(":/img/images/led_on.png"));
    else
        ui->VNA_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));
}

void scanner_gui::on_scan_settings_button_clicked()
{
    emit insthread_stop();
    RS_Instruments *sa;
    RS_Instruments *vna;
    if(vna_connected_bool || sa_connected_bool){
        if(sa_connected_bool && vna_connected_bool){
            QMessageBox *msgBox = new QMessageBox;
            msgBox->setMinimumSize(400,200);
            msgBox->setWindowTitle("Measurement instrument");
            msgBox->setText("Both, SA and VNA connected!");
            msgBox->setInformativeText("Which one do you want to use?");
            QAbstractButton *SA = msgBox->addButton(("SA"), QMessageBox::YesRole);
            QAbstractButton *VNA = msgBox->addButton(("VNA"), QMessageBox::NoRole);
            msgBox->setIcon(QMessageBox::Question);
            msgBox->exec();

            if(msgBox->clickedButton() == SA){
                sa = new RS_Instruments(RS_Instruments::SA, sa_ip_address);
                if(sa->tcp_connect()){
                    connect(sa, &RS_Instruments::stop_scan, this, &scanner_gui::current_scan_stopped);
                    robot->assign_rs_instrument(sa);
                    wo_vna = true;
                }else{
                    QMessageBox::critical(this, "SA connection error!", "Error during the connection atempt!");
                    return;
                }
                delete msgBox;
            }else if(msgBox->clickedButton() == VNA){
                vna = new RS_Instruments(RS_Instruments::VNA, vna_ip_address);
                if(vna->tcp_connect()){
                    connect(vna, &RS_Instruments::stop_scan, this, &scanner_gui::current_scan_stopped);
                    robot->assign_rs_instrument(vna);
                    wo_sa = true;
                }else{
                    QMessageBox::critical(this, "VNA connection error!", "Error during the connection atempt!");
                    return;
                }
                delete msgBox;
            }

        }else{
            if(sa_connected_bool){
                sa = new RS_Instruments(RS_Instruments::SA, sa_ip_address);
                if(sa->tcp_connect()){
                    connect(sa, &RS_Instruments::stop_scan, this, &scanner_gui::current_scan_stopped);
                    robot->assign_rs_instrument(sa);
                    wo_vna = true;
                }else{
                    QMessageBox::critical(this, "SA connection error!", "Error during the connection atempt!");
                    return;
                }
            }
            if(vna_connected_bool){
                vna = new RS_Instruments(RS_Instruments::VNA, vna_ip_address);
                if(vna->tcp_connect()){
                    connect(vna, &RS_Instruments::stop_scan, this, &scanner_gui::current_scan_stopped);
                    robot->assign_rs_instrument(vna);
                    wo_sa = true;
                }else{
                    QMessageBox::critical(this, "VNA connection error!", "Error during the connection atempt!");
                    return;
                }
            }
        }
        if((vna_connected_bool || sa_connected_bool)){
            if(wo_vna){
                qDebug("No vna");
                ins = sa;
            }else if(wo_sa){
                qDebug("No sa");
                ins = vna;
            }
            robot->assign_rs_instrument(ins);

            scan_settings scan_settings(ins, wo_vna, this);
            connect(ins, SIGNAL(stop_scan()), this, SLOT(current_scan_stopped()));

            if(scan_wizard_started)
                connect(ins, SIGNAL(stop_scan()), wizard, SLOT(scan_aborted()));

            scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
            scan_settings.setModal(true);
            scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
            scan_settings.exec();

            ins->get_datapath(current_scan_datapath);
            ins->step_size = step_size;
            wo_sa = false;
            wo_vna = false;
            ins_created = true;
            emit instruments_created();
        }
    }else{
        QMessageBox::critical(this, "No instrument", "No measurement instrument connected!");
    }
}

void scanner_gui::on_datasave_test_clicked()
{
    //QDesktopServices::openUrl(QUrl::fromLocalFile("C:/Users/Near-field scanner/Desktop/GUI user comments.txt"));
}


//Robot control
void scanner_gui::disp_robot_msg(QString msg)
{
    ui->robotTerminal->setText(msg);
}

void scanner_gui::on_stepsize_xy_valueChanged(double arg1)
{
    if(scan_wizard_started){
        scan_area->step_size = arg1;
        step_size = arg1;
    }
}

void scanner_gui::on_stepsize_z_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
}

void scanner_gui::on_scan_height_valueChanged(double arg1)
{
    robot->scan_heigh_update(arg1);
}

void scanner_gui::on_Y_plus_button_pressed()
{
    robot->Y_shift(-ui->stepsize_xy->value());
}

void scanner_gui::on_Y_minus_button_pressed()
{
    robot->Y_shift(ui->stepsize_xy->value());
}

void scanner_gui::on_X_plus_button_pressed()
{
    robot->X_shift(-ui->stepsize_xy->value());
}

void scanner_gui::on_X_minus_button_pressed()
{
    robot->X_shift(ui->stepsize_xy->value());
}

void scanner_gui::on_Z_plus_pressed()
{
    robot->Z_shift(ui->stepsize_z->value());
    robot->ask_for_camera_height();
}

void scanner_gui::on_Z_minus_pressed()
{
    robot->Z_shift(-ui->stepsize_z->value());
    robot->ask_for_camera_height();
}

void scanner_gui::on_home_button_clicked()
{
    robot->goto_home_pos();
}

void scanner_gui::on_rotate_probe_button_clicked()
{
    robot->probe_rotate();
}

//Main settings
void scanner_gui::on_actionSettings_triggered()
{
    MainSettings mainsettings(nullptr, robot);
    connect(&mainsettings, SIGNAL(send_ref00_offsets(double, double)), videothread, SLOT(get_ref00_offsets(double, double)));
    mainsettings.setWindowFlags(mainsettings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    mainsettings.setModal(true);
    mainsettings.setFixedSize(mainsettings.width(),mainsettings.height());
    mainsettings.exec();
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
    robot->tool_change(t_x, t_y, t_z);
}

void scanner_gui::tools_init()
{
    QFile inputFile(QCoreApplication::applicationDirPath() + "/settings//tooldata.txt");
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
    robot->stop_scan();
    robot->goto_home_pos();
}

