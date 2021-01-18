
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
#include <QCameraViewfinder>
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
    timer2 = new QTimer;
    connect(timer2, &QTimer::timeout, this, &scanner_gui::ask_for_cam_h);
}

scanner_gui::~scanner_gui()
{
    //delete videothread;
    delete wizard;
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
    connect(ui->lastImagePreviewLabel, SIGNAL(sendQrect(QRect&)), scan_area, SLOT(receive_cropped_area(QRect&)));

    //connect(ui->liveStream, SIGNAL(sendPos(int, int)), videothread, SLOT(mark_scanheight(int ,int)));

    connect(robot, SIGNAL(allow_emit_pos(bool)), ui->liveStream, SLOT(allow_emit(bool)));
    // Delete last scan settings file
    QFile file(QCoreApplication::applicationDirPath() + "/scansettings.ini");
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
    if(robot->tcp_connect())
    {
        ui->robot_connect_button->setEnabled(false);
        ui->robot_connect_button->setText("Connected");
        ui->robotTerminal->setText("");
        ui->robotTerminal->setText("Connected to Kawasaki F Controller");
        ui->robotManualControl_frame->setEnabled(true);
    }
    else
    {
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
    connect(videothread, SIGNAL(positions(bool, int, int, int, int, int, int)), pcb,
            SLOT(cv_getcoord(bool, int, int, int, int, int, int)));
    connect(videothread, SIGNAL(error(QString)), this, SLOT(cameraError(QString)));
    connect(videothread, SIGNAL(cameraOpened()), this, SLOT(cameraConnected()));
    connect(ui->camera_focus_dial, SIGNAL(valueChanged(int)), videothread, SLOT(refocus(int)));
    connect(ui->camera_contrast_dial, SIGNAL(valueChanged(int)), videothread, SLOT(recontrast(int)));
    connect(ui->camera_brightness_dial, SIGNAL(valueChanged(int)), videothread, SLOT(rebrightness(int)));
    connect(robot, SIGNAL(send_area_to_videothread(qint64)), videothread, SLOT(receive_area(qint64)));
    connect(robot, SIGNAL(stop_displaying_point()), videothread, SLOT(height_measurement_done()));
//    connect(videothread, SIGNAL(height_scan_point_error()), this, SLOT(throw_height_meas_error()));
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
void scanner_gui::Take_img_button_clicked()
{
    displayCapturedImage();
    processCapturedImage(lastImage);
}

void scanner_gui::resetCamera_button_clicked()
{
    displayViewfinder();
}

void scanner_gui::cameraConnected()
{
    ui->actionReset_Camera->setEnabled(false);
    ui->actionReset_Camera->setText("Connected");
}

void scanner_gui::cameraError(QString error)
{
    ui->actionReset_Camera->setEnabled(true);
    ui->actionReset_Camera->setText("Connect");
    ui->liveStream->setText("Camera error. Try to reconect!");
    QMessageBox::critical(this, "Camera error", error);
//    qApp->quit();
//    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
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

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    QString path = current_scan_datapath + "/cropped_image.PNG";
    scaledImage.save(path, "PNG",100);
}

void scanner_gui::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void scanner_gui::on_actionReset_Camera_triggered()
{
    video_thread_init();
}

void scanner_gui::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void scanner_gui::throw_height_meas_error()
{
    QMessageBox::warning(this, "Point outside PCB", "Selected point is outside the PCB.\nSelect a new point!");
}


//Scan wizard functions
void scanner_gui::on_Start_scan_button_clicked()
{
    if(robot->_socket_robot->state() == QAbstractSocket::ConnectedState)
    {

        pcb = new DUT_size;
        scan_area = new DUT_size;
        robot->assign_duts(pcb, scan_area);

        connect(robot, SIGNAL(send_area_to_videothread()), pcb, SLOT(send_area_request()));
        //connect(videothread, SIGNAL(send_scanheight_point(int, int)), pcb, SLOT(receive_scanheight_point(int, int)));
        wizard = new ScanWizard(this);
        //connect(wizard, SIGNAL(detect_pcb(bool)), videothread, SLOT(start_detection(bool)));
        //connect(videothread, SIGNAL(pcb_found()), wizard, SLOT(pcb_found()));
        connect(pcb, SIGNAL(send_coord_to_wizard(QPoint, QRect)), wizard, SLOT(take_coord(QPoint, QRect)));
        connect(wizard, SIGNAL(send_robot_to_origin(bool)), this, SLOT(wizard_robot_to_origin(bool)));
        //connect(wizard, SIGNAL(scan_area_origin_detect(bool)), videothread, SLOT(scan_origin_detect(bool)));
        connect(wizard, SIGNAL(set_scan_settings(int)), this, SLOT(wizard_mark_background(int)));
        connect(wizard, SIGNAL(run_scan(bool)), this, SLOT(wizard_scan_control(bool)));
        connect(wizard, SIGNAL(send_for_2nd_takepic()), this, SLOT(send_to_takepic2_pos()));
        connect(wizard, SIGNAL(ask_for_cam_height()), this, SLOT(ask_robot_for_cam_height()));
        connect(robot, SIGNAL(height_measured()), wizard, SLOT(height_measure_finished()));
        connect(robot, SIGNAL(scan_finished_to_wizard()), wizard, SLOT(scan_finished()));
        connect(wizard, SIGNAL(allow_emit_pos(bool)), ui->liveStream, SLOT(allow_emit(bool)));
        connect(this, SIGNAL(instruments_created()), wizard, SLOT(inst_created()));

        wizard->setWindowFlag(Qt::WindowStaysOnTopHint);
        wizard->adjustSize();
        //wizard->setWindowFlag(Qt::FramelessWindowHint);
        wizard->setWindowTitle("Scan Wizard");
        wizard->move(1300,100);
        wizard->setStyleSheet("background-color: rgba(180,210,210,1)");
        wizard->show();
    }
    else
    {
        QMessageBox::critical(this, "Critial error!", "Scan cannot be performed when the robot is offline. Turn on the robot, connect using button on the right toolbar and try again!");
    }
    wizard_mark_background(10);
}

void scanner_gui::wizard_robot_to_origin(bool height_scan)
{
    if(height_scan){
        robot->goto_meas_height(pcb->scan_height_point.x(), pcb->scan_height_point.y());
    }else{
        robot->goto_origin(pcb->corner.x()-scan_area->corner.x(),
                           pcb->corner.y()-scan_area->corner.y());
    }
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

void scanner_gui::wizard_scan_control(bool run)
{
    if(run)
    {
        set_scan_step_sizes();
        start_scan();
    }
    else
    {
        stop_scan();
    }
}


//Scan control functions
void scanner_gui::start_scan()
{
    current_scan_datapath = datapath + "SCAN_" + QDate::currentDate().toString("dd_MM_yyyy") + "__" + QTime::currentTime().toString("hh_mm_ss") + "/";
    if(!QDir(current_scan_datapath).exists())
        QDir().mkdir(current_scan_datapath);

    emit insthread_stop();

    get_trace_data(time_for_amplitude);
    for(int i=0; i<3000; i++){};

    robot->start_scan();
}

void scanner_gui::stop_current_scan()
{
    robot->stop_scan();
    delete pcb;
    delete scan_area;
    delete sa;
    delete vna;
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

void scanner_gui::confirm_written_bytes(qint64 b)
{
    qDebug() << "Bytes written for SA: " << b;
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
    time_for_amplitude = false;
    if(vna_connected_bool || sa_connected_bool)
    {
        bool wo_vna = false;
        bool wo_sa = false;

        if(sa_connected_bool)
        {
            sa = new RS_Instruments(RS_Instruments::SA);
            connect(sa, &RS_Instruments::stop_scan, this, &scanner_gui::stop_current_scan);
            robot->assign_rs_instrument(sa);
        }
        else
        {
            QMessageBox::StandardButton reply = QMessageBox::warning(this, "Spectrum Analyzer", "SA not connected. Continue?", QMessageBox::Yes | QMessageBox::No);
            if(reply == QMessageBox::Yes)
                wo_sa = true;
        }
        if(vna_connected_bool || wo_sa)
        {
            if(vna_connected_bool)
            {
                vna = new RS_Instruments(RS_Instruments::SA);
                connect(vna, &RS_Instruments::stop_scan, this, &scanner_gui::stop_current_scan);
                robot->assign_rs_instrument(vna);
            }
            else
            {
                QMessageBox::StandardButton reply = QMessageBox::warning(this, "VNA Analyzer", "VNA not connected. Continue?", QMessageBox::Yes | QMessageBox::No);
                if(reply == QMessageBox::Yes)
                    wo_vna = true;
            }
        }

        if((vna_connected_bool && sa_connected_bool) || wo_sa || wo_vna)
        {
            if(wo_vna)
            {
                qDebug("No vna");
                robot->assign_rs_instrument(sa);
                scan_settings scan_settings(sa, false, this);
                connect(&scan_settings, SIGNAL(send_sweep_points_amount(int)), this, SLOT(get_sweep_points_amount(int)));
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();

            }
            else if(wo_sa)
            {
                qDebug("No sa");
                robot->assign_rs_instrument(vna);
                scan_settings scan_settings(vna, true, this);
                connect(&scan_settings, SIGNAL(send_sweep_points_amount(int)), this, SLOT(get_sweep_points_amount(int)));
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();
            }

            emit instruments_created();
            instrument_thread_init();
        }
    }
    else
    {
        QMessageBox::critical(this, "No instrument", "No measurement instrument connected!");
    }
}

void scanner_gui::get_sweep_points_amount(int sp)
{
    sweep_points = sp;
}

void scanner_gui::on_datasave_test_clicked()
{
    get_trace_data(false);
}

void scanner_gui::vna_dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;

    b_data.append(_socket_vna->readAll());

    if(b_data.size() > sweep_points*4)
    {
        if(time_for_amplitude)
        {
            no[0] = b_data.at(1);
            digits = atoi(no);
            qDebug() << "Digits: " << digits;
            for(int i=0; i<digits; i++)
            {
                data.append(b_data.at(2+i));
            }
            data=data.left(digits);
            bytes = data.toUInt();
            qDebug() << "Bytes: " << bytes;

            if(bytes == sweep_points*4)
            {
                b_data.chop(1);
                b_data=b_data.right(bytes);
                const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                std::vector<float> mag;
                for (int i=0; i<b_data.size()/4; ++i)
                {
                    float d = *ptrFloat;
                    mag.push_back(d);
                    ptrFloat++;
                }

                temp2d.push_back(mag);
                qDebug() << "Saved magnitude vector of size: " << mag.size() << "To temp2d vector of size: " << temp2d.size();
                qDebug() << "Current point: " << "(" << save_x << ":" << save_y << ")";

                if(save_x == scan_columns)
                {
                    data_tensor.push_back(temp2d);
                    temp2d.push_back(mag);
                    current_scan_point_x=-1;
                    temp2d.clear();
                    qDebug() << "Pushed back to data_tensor vector of size: " << data_tensor.size();
                }
                b_data.clear();
                digits = 0;
                bytes = 0;
            }
            else
            {
                stop_scan();
                QMessageBox::critical(this, "Scan error!", "Data save error! Scan aborted!");
            }
        }
        else
        {
            no[0] = b_data.at(1);
            digits = atoi(no);
            qDebug() << "Digits: " << digits;
            for(int i=0; i<digits; i++)
            {
                data[i] = b_data.at(2+i);
            }
            bytes = atoi(data);
            qDebug() << "Bytes: " << bytes;

            if(bytes == sweep_points*4)
            {
                b_data.chop(1);
                b_data=b_data.right(bytes);
                const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                std::vector<float> mag;
                for (int i=0; i<b_data.size()/4; ++i)
                {
                    float d = *ptrFloat;
                    mag.push_back(d);
                    ptrFloat++;
                }

                QString path = current_scan_datapath + "xaxis_data.bin";
                std::ofstream file(path.toLocal8Bit(), std::ios::binary);
                if(file.is_open())
                {
                    file.write(reinterpret_cast<const char*>(&mag[0]), mag.size()*sizeof(float));
                    file.close();
                }
                qDebug() << "Freq saved";
                b_data.clear();
                mag.clear();
                time_for_amplitude = true;
                digits = 0;
                bytes = 0;
                return;
            }
            else
            {
                stop_scan();
                QMessageBox::critical(this, "Scan error!", "Data save error! Scan aborted!");
            }
        }
    }
}


//Robot
void scanner_gui::disp_robot_msg(QString msg)
{
    ui->robotTerminal->setText(msg);
}

void scanner_gui::send_to_takepic2_pos()
{
    robot->goto_takepic2_pos(pcb_size.width()/2, pcb_size.width()/2);
}

void scanner_gui::ask_robot_for_cam_height()
{
    robot->ask_for_camera_height();
}

void scanner_gui::ask_for_cam_h()
{

}

void scanner_gui::on_stepsize_xy_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
//    QString mystring = "xshift = %1\n";
//    mystring = mystring.arg(QString::number(ui->stepsize_xy->value()));
//    _socket_robot->write(mystring.toLocal8Bit());
//    _socket_robot->waitForBytesWritten(30);
//    mystring = "yshift = %1\n";
//    mystring = mystring.arg(QString::number(ui->stepsize_xy->value()));
//    _socket_robot->write(mystring.toLocal8Bit());
//    _socket_robot->waitForBytesWritten(30);
}

void scanner_gui::on_stepsize_z_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
//    QString mystring = "zshift = %1\n";
//    mystring = mystring.arg(QString::number(ui->stepsize_z->value()));
//    _socket_robot->write(mystring.toLocal8Bit());
//    _socket_robot->waitForBytesWritten(30);
}

void scanner_gui::on_scan_height_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    QString mystring = "measuring_height = %1\n";
    mystring = mystring.arg(QString::number(ui->scan_height->value()));
    _socket_robot->write(mystring.toLocal8Bit());
}

void scanner_gui::on_Y_plus_button_pressed()
{
    QString msg = "yShift = %1\n";
    msg = msg.arg(QString::number(-ui->stepsize_xy->value()));
    _socket_robot->write(msg.toLocal8Bit());
}

void scanner_gui::on_Y_minus_button_pressed()
{
    QString msg = "yShift = %1\n";
    msg = msg.arg(QString::number(ui->stepsize_xy->value()));
    _socket_robot->write(msg.toLocal8Bit());
}

void scanner_gui::on_X_plus_button_pressed()
{
    QString msg = "xShift = %1\n";
    msg = msg.arg(QString::number(-ui->stepsize_xy->value()));
    _socket_robot->write(msg.toLocal8Bit());
}

void scanner_gui::on_X_minus_button_pressed()
{
    QString msg = "xShift = %1\n";
    msg = msg.arg(QString::number(ui->stepsize_xy->value()));
    _socket_robot->write(msg.toLocal8Bit());
}

void scanner_gui::on_Z_plus_pressed()
{
    QString msg = "zShift = %1\n";
    msg = msg.arg(QString::number(ui->stepsize_z->value()));
    _socket_robot->write(msg.toLocal8Bit());
    ask_for_cam_h();
}

void scanner_gui::on_Z_minus_pressed()
{
    QString msg = "zShift = %1\n";
    msg = msg.arg(QString::number(-ui->stepsize_z->value()));
    _socket_robot->write(msg.toLocal8Bit());
    ask_for_cam_h();
}

void scanner_gui::on_home_button_clicked()
{
    _socket_robot->write("takepic = 1\n");
    camera_distance_2 = camera_distance;
}

void scanner_gui::robotBytesWritten(qint64 b)
{
    Q_UNUSED(b);
}

void scanner_gui::closeEvent (QCloseEvent *event)
{
    Q_UNUSED(event);
//    _socket_robot->write("mes_abort = 1\n");
//    _socket_robot->waitForBytesWritten(30);
//    _socket_robot->write("takepic = 1\n");
//    _socket_robot->waitForBytesWritten(30);
}


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

    ///Set tl_x,tl_y,tl_z to the appropriate values
    QString msg = "tl_x = %1\n";
    msg = msg.arg(QString::number(t_x));
    _socket_robot->write(msg.toLocal8Bit());
    _socket_robot->waitForBytesWritten(20);

    msg = "tl_y = %1\n";
    msg = msg.arg(QString::number(t_y));
    _socket_robot->write(msg.toLocal8Bit());
    _socket_robot->waitForBytesWritten(20);

    msg = "tl_z = %1\n";
    msg = msg.arg(QString::number(t_z));
    _socket_robot->write(msg.toLocal8Bit());
    _socket_robot->waitForBytesWritten(20);

    ///Call the robot's toolchange function
    _socket_robot->write("tool_change = 1\n");
    _socket_robot->waitForBytesWritten(20);

}

void scanner_gui::tools_init(){


    QFile inputFile(QCoreApplication::applicationDirPath() + "/tooldata.txt");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList fields = line.split(";");
            Tool* T = new Tool(fields[0],fields[1].toFloat(),fields[2].toFloat(),fields[3].toFloat());
            ///qDebug() << fields;
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

    ///qDebug("Entering Tool panel");
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
        ///qDebug() << Tools[i]->tool_name;
    }
}

