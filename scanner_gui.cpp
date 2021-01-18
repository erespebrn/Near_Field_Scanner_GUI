
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

//Constructor destructor
scanner_gui::scanner_gui() : ui(new Ui::scanner_gui)
{
    ui->setupUi(this);
    timer2 = new QTimer;
    connect(timer2, &QTimer::timeout, this, &scanner_gui::ask_for_cam_h);
}

scanner_gui::~scanner_gui()
{
    delete _socket_sa;
    delete _socket_robot;
    delete videothread;
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
    connect(ui->liveStream, SIGNAL(sendPos(int, int)), videothread, SLOT(mark_scanheight(int ,int)));
    connect(this, SIGNAL(allow_emit_pos(bool)), ui->liveStream, SLOT(allow_emit(bool)));
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
    QString send_msg = "";
    // *** Robot TCP connection *** //
    _socket_robot = new QTcpSocket(this);
    _socket_robot->connectToHost(robot_ip_address, 23);
    _socket_robot->waitForConnected();
    if(_socket_robot->state() == QAbstractSocket::ConnectedState)
    {
       ui->robot_connect_button->setEnabled(false);
       ui->robot_connect_button->setText("Connected");
       ui->robotTerminal->setText("");
       ui->robotTerminal->setText("Connected to Kawasaki F Controller");
       ui->robotManualControl_frame->setEnabled(true);

       send_msg = "";
       _socket_robot->write(send_msg.toLocal8Bit());
       _socket_robot->waitForBytesWritten(40);

       send_msg = "as\n";
       _socket_robot->write(send_msg.toLocal8Bit());
       _socket_robot->waitForBytesWritten(20);

       QThread::sleep(1);

       send_msg = "ZPOWER ON\n";
       _socket_robot->write(send_msg.toLocal8Bit());
       _socket_robot->waitForBytesWritten(20);

       send_msg = "EXECUTE main\n";
       _socket_robot->write(send_msg.toLocal8Bit());
       // *** //

       connect(_socket_robot, SIGNAL(readyRead()), this, SLOT(read_robot_msg()));
       connect(_socket_robot, SIGNAL(bytesWritten(qint64)), this, SLOT(robotBytesWritten(qint64)));
    }
    else
    {
       // QMessageBox::warning(this, "Robot error", "Robot not connected!");
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
    connect(videothread, SIGNAL(positions(bool, int, int, int, int, int, int)), this,
            SLOT(cv_getcoord(bool, int, int, int, int, int, int)));
    connect(videothread, SIGNAL(error(QString)), this, SLOT(cameraError(QString)));
    connect(videothread, SIGNAL(cameraOpened()), this, SLOT(cameraConnected()));
    connect(ui->camera_focus_dial, SIGNAL(valueChanged(int)), videothread, SLOT(refocus(int)));
    connect(ui->camera_contrast_dial, SIGNAL(valueChanged(int)), videothread, SLOT(recontrast(int)));
    connect(ui->camera_brightness_dial, SIGNAL(valueChanged(int)), videothread, SLOT(rebrightness(int)));
    connect(this, SIGNAL(send_area_to_videothread(qint64)), videothread, SLOT(receive_area(qint64)));
    connect(videothread, SIGNAL(send_scanheight_point(int, int)), this, SLOT(receive_scanheight_point(int, int)));
    connect(this, SIGNAL(stop_displaying_point()), videothread, SLOT(height_measurement_done()));
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

void scanner_gui::cv_getcoord(bool scan, int o_x, int o_y, int pcb_x, int pcb_y, int pcb_w, int pcb_h)
{
    origin = QPoint(o_x, o_y);

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float width_cropped = (camera_distance*pcb_w*sensor_width/(focal_lenght*1280));
    float height_cropped = (camera_distance*pcb_h*sensor_height/(focal_lenght*960));

    float x_dist_px = pcb_x - origin.x();
    float y_dist_px = pcb_y - origin.y();

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280));
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960));

//    x_dist_mm += 10;

    if(!scan)
    {
        pcb_corner = QPoint(int(round(x_dist_mm)), int(round(y_dist_mm)));
        pcb_size = QRect(pcb_corner.x(), pcb_corner.y(), int(round(width_cropped)), int(round(height_cropped)));
        emit send_coord_to_wizard(pcb_corner, pcb_size);
    }
    else
    {
        scan_pcb_corner = QPoint(o_x, o_y);
    }
}

void scanner_gui::processCapturedImage(const QImage &img)
{
    QImage scaledImage = img.scaled(ui->lastImagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    displayCapturedImage();
}

void scanner_gui::displayCroppedImage(QRect &rect)
{
    croppedOrigin = rect;
    const QPixmap* pixmap = ui->lastImagePreviewLabel->pixmap();
    QImage image( pixmap->toImage() );
    QImage cropped = image.copy(rect);
    QImage scaledImage = cropped.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    scaledImage.save(QDir::toNativeSeparators(QDir::homePath() + "/Pictures/cropped_image.PNG"), "PNG",100);

    qDebug() << "Marked area: " << rect.width() * rect.height();

    // *** Determine the real size of the object on an image *** //
    //Since the image taken is scaled, scale factor must be used
//    float scale_factor_x = 1.14*((float)resolution_max_width/(float)ui->lastImagePreviewLabel->width());
//    float scale_factor_y = 0.92*((float)resolution_max_height/(float)ui->lastImagePreviewLabel->height());

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float width_cropped = ((float)camera_distance_2*(float)rect.width()*sensor_width/(focal_lenght*1280));
    float height_cropped = ((float)camera_distance_2*(float)rect.height()*sensor_height/(focal_lenght*960));

    float x_dist_px = scan_pcb_corner.x() - croppedOrigin.x();
    float y_dist_px = scan_pcb_corner.y() - croppedOrigin.y();

    float x_dist_mm = ((float)camera_distance_2*x_dist_px*sensor_width/(focal_lenght*1280));
    float y_dist_mm = ((float)camera_distance_2*y_dist_px*sensor_height/(focal_lenght*960));

    scan_area_size_px = QRect(x_dist_px, y_dist_px, rect.width(), rect.height());
    scan_area_corner = QPoint(int(round(x_dist_mm)), int(round(y_dist_mm)));
    scan_area_size = QRect(scan_area_corner.x(), scan_area_corner.y(), int(round(width_cropped)), int(round(height_cropped)));

    qDebug() << "Scan area size: " << scan_area_size;
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

void scanner_gui::receive_scanheight_point(int x, int y)
{
    float x_dist_px = x - origin.x();
    float y_dist_px = y - origin.y();

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280));
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960));

    scan_height_point = QPoint(x_dist_mm+18, y_dist_mm);
}

void scanner_gui::throw_height_meas_error()
{
    QMessageBox::warning(this, "Point outside PCB", "Selected point is outside the PCB.\nSelect a new point!");
}


//Scan wizard functions
void scanner_gui::on_Start_scan_button_clicked()
{
    if(_socket_robot->state() == QAbstractSocket::ConnectedState)
    {
        data_tensor.clear();
        b_data.clear();
        temp2d.clear();
        freq.clear();
        current_scan_point_x=-1;
        scan_rows = 0;
        scan_columns = 0;
        save_x = 0;
        save_y = 0;
        camera_distance_2 = 10000;

        _socket_robot->write("takepic = 1\n");
        _socket_robot->waitForBytesWritten(1000);

        wizard = new ScanWizard(this);
        connect(wizard, SIGNAL(detect_pcb(bool)), videothread, SLOT(start_detection(bool)));
        connect(videothread, SIGNAL(pcb_found()), wizard, SLOT(pcb_found()));
        connect(this, SIGNAL(send_coord_to_wizard(QPoint, QRect)), wizard, SLOT(take_coord(QPoint, QRect)));
        connect(wizard, SIGNAL(send_robot_to_origin(bool)), this, SLOT(wizard_robot_to_origin(bool)));
        connect(wizard, SIGNAL(scan_area_origin_detect(bool)), videothread, SLOT(scan_origin_detect(bool)));
        connect(wizard, SIGNAL(set_scan_settings(int)), this, SLOT(wizard_mark_background(int)));
        connect(wizard, SIGNAL(run_scan(bool)), this, SLOT(wizard_scan_control(bool)));
        connect(wizard, SIGNAL(send_for_2nd_takepic()), this, SLOT(send_to_top_pcb_edge()));
        connect(wizard, SIGNAL(ask_for_cam_height()), this, SLOT(ask_robot_for_cam_height()));
        connect(this, SIGNAL(height_measured()), wizard, SLOT(height_measure_finished()));
        connect(this, SIGNAL(scan_finished_to_wizard()), wizard, SLOT(scan_finished()));
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
        QMessageBox::critical(this, "Critial error!", "Scan cannot be performed when robot is offline. Turn on the robot, connect using button on the right toolbar and try again!");
    }
    wizard_mark_background(10);
}

void scanner_gui::wizard_robot_to_origin(bool middle)
{
    if(middle)
    {
        send_robot_coordinates(middle);
        _socket_robot->write("mesheight = 1\n");
        _socket_robot->waitForBytesWritten(10);
    }
    else
    {
        send_robot_coordinates(middle);
        _socket_robot->write("Goto_Origin = 1\n");
        _socket_robot->waitForBytesWritten();
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
void scanner_gui::stop_scan_button_clicked()
{
    _socket_robot->write("mes_abort = 1\n");
    _socket_robot->waitForBytesWritten(30);
    _socket_robot->write("takepic = 1\n");
    _socket_robot->waitForBytesWritten(30);
}

void scanner_gui::start_scan()
{
    current_scan_datapath = datapath + "SCAN_" + QDate::currentDate().toString("dd_MM_yyyy") + "__" + QTime::currentTime().toString("hh_mm_ss") + "/";
    if(!QDir(current_scan_datapath).exists())
        QDir().mkdir(current_scan_datapath);

    emit insthread_stop();
    get_trace_data(time_for_amplitude);
    for(int i=0; i<3000; i++){};
    _socket_robot->write("Mes = 1\n");
    _socket_robot->waitForBytesWritten(20);
}

void scanner_gui::stop_scan()
{
    _socket_robot->write("mes_abort = 1\n");
    _socket_robot->waitForBytesWritten(20);
    _socket_robot->write("takepic = 1\n");
    _socket_robot->waitForBytesWritten(20);
    scan_point = 0;
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
        QString msg = "";
        bool wo_vna = false;
        bool wo_sa = false;
        _socket_sa = new QTcpSocket;

        connect(_socket_sa, SIGNAL(readyRead()), this, SLOT(sa_dataread()));
        connect(_socket_sa, SIGNAL(bytesWritten(qint64)), this, SLOT(confirm_written_bytes(qint64)));

        if(sa_connected_bool)
        {
            _socket_sa->connectToHost(sa_ip_address, 5025);
            _socket_sa->waitForConnected(10);


            if(_socket_sa->state() == QAbstractSocket::ConnectedState)
            {
                msg = "*RST\n";
                _socket_sa->write(msg.toLocal8Bit());
                _socket_sa->waitForBytesWritten();
                msg = "";

                msg = "*ESE 1\n";
                _socket_sa->write(msg.toLocal8Bit());
                _socket_sa->waitForBytesWritten();
                msg = "";

                msg = "SYST:DISP:UPD ON\n";
                _socket_sa->write(msg.toLocal8Bit());
                _socket_sa->waitForBytesWritten();
                msg = "";

                _socket_sa->write("FORM:DATA REAL,32\n");
                _socket_sa->waitForBytesWritten(20);

                _socket_sa->write("INIT:CONT OFF\n");
                _socket_sa->waitForBytesWritten(20);
            }
        }
        else
        {
            QMessageBox::StandardButton reply = QMessageBox::warning(this, "Spectrum Analyzer", "SA not connected. Continue?", QMessageBox::Yes | QMessageBox::No);

            if(reply == QMessageBox::Yes)
                wo_sa = true;
        }

        if(sa_connected_bool || wo_sa)
        {
            if(vna_connected_bool)
            {
                emit insthread_stop();
                _socket_vna->connectToHost(vna_ip_address, 5025);
                _socket_vna->waitForConnected(10);

                if(_socket_vna->state() == QAbstractSocket::ConnectedState)
                {
//                    msg = "SYST:TSL OFF\n";
//                    _socket_vna.write(msg.toLocal8Bit());
//                    _socket_vna.waitForBytesWritten();
//                    msg = "";

//                    msg = "SYST:TSL SCR\n";
//                    _socket_vna.write(msg.toLocal8Bit());
//                    _socket_vna.waitForBytesWritten();
//                    msg = "";

//                    msg = "SYST:DISP:BAR:STO OFF\n";
//                    _socket_vna.write(msg.toLocal8Bit());
//                    _socket_vna.waitForBytesWritten();
//                    msg = "";

//                    msg = "SYST:DISP:UPD ON\n";
//                    _socket_vna.write(msg.toLocal8Bit());
//                    _socket_vna.waitForBytesWritten();
//                    msg = "";
                }
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
            if(vna_connected_bool && sa_connected_bool && !wo_sa && !wo_vna)
            {
                qDebug("All");
                scan_settings scan_settings(_socket_sa, _socket_vna, this);
                connect(&scan_settings, SIGNAL(send_sweep_points_amount(int)), this, SLOT(get_sweep_points_amount(int)));
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();
            }
            else if(wo_vna)
            {
                qDebug("No vna");
                scan_settings scan_settings(_socket_sa, false, this);
                connect(&scan_settings, SIGNAL(send_sweep_points_amount(int)), this, SLOT(get_sweep_points_amount(int)));
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();

            }
            else if(wo_sa)
            {
                qDebug("No sa");
                scan_settings scan_settings(_socket_vna, true, this);
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

void scanner_gui::get_trace_data(bool)
{
    if(time_for_amplitude)
    {
        _socket_sa->write("DISP:TRAC1:MODE WRIT\n");
        _socket_sa->write("DISP:TRAC1:MODE MAXH\n");
        _socket_sa->write("INIT;*WAI\n");
        _socket_sa->write("TRAC:DATA? TRACE1;*WAI\n");
    }
    else
    {
        _socket_sa->write("TRAC:DATA:X? TRACE1\n");
    }
}

void scanner_gui::sa_dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;

    b_data.append(_socket_sa->readAll());

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
                QMessageBox::critical(this, "Scan error!", "Data save error! Scan aborted!");
                stop_scan();
            }
        }
    }
}


//Robot control functions
void scanner_gui::read_robot_msg()
{
    QByteArray welcome_msg;
    QByteArray msg;
    uint8_t i = 0;

    if(robot_first_run)
    {
        welcome_msg.append(_socket_robot->readAll());

        if(welcome_msg.at(welcome_msg.size()-1) == '>')
        {
            qDebug() << welcome_msg;
            robot_first_run = false;
        }
    }

    if(_socket_robot->bytesAvailable() && !robot_first_run)
    {
        robot_raw_data.append(_socket_robot->readAll());

        if(robot_raw_data.at(robot_raw_data.size()-1) == '\n')
        {
            if(!robot_raw_data.isEmpty())
                qDebug() << "Robot raw message: " << robot_raw_data;

            bool time_for_msg = false;
            for(int t=0; t<robot_raw_data.size(); t++)
            {
                char a = robot_raw_data.at(t);
                if(a == '@')
                    time_for_msg = true;
                if(time_for_msg)
                    msg.append(robot_raw_data.at(t));
            }
            if(!msg.isEmpty())
            {
                char arg_to_cvt[5];
                arg_to_cvt[0] = msg.at(1);
                arg_to_cvt[1] = msg.at(2);
                arg_to_cvt[2] = '\n';
                i = atoi(arg_to_cvt);
            }
            robot_raw_data.clear();
        }
    }

    switch(i)
    {
        case 1:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Scan started");
            break;
        case 2:
        {
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Scan in progress...");

            QByteArray col_to_cvt;
            QByteArray row_to_cvt;

            bool time_for_col = false;
            for(size_t p=3; p<strlen(msg); p++)
            {
                char a = msg.at(p);
                if(a == ' ')
                    time_for_col = true;
                if(!time_for_col)
                {
                    row_to_cvt.append(a);
                }
                if(time_for_col)
                {
                    col_to_cvt.append(a);
                }
            }

            save_x = col_to_cvt.toUInt();
            save_y = row_to_cvt.toUInt();

            if(sa_connected_bool)
                get_trace_data(true);
            break;
        }
        case 3:
        {
            if(y_comp)
            {
                ui->robotTerminal->setText("");
                ui->robotTerminal->setText("Y comp Scan finished!");

                current_scan_point_x = -1;
                QString path = current_scan_datapath+"y_comp_scan_data_tensor.bin";
                std::ofstream file;

                file.open(path.toLocal8Bit(), std::ios::binary);

                if(file)
                {
                    qDebug() << "File does not exists. Created file!";
                    float sp = (float)sweep_points;
                    float x_max = (float)scan_columns;
                    float y_max = (float)scan_rows;

                    float step_size_px = (float)scan_area_size_px.width()/scan_rows;
                    float scan_width_px = (float)scan_area_size_px.width();
                    float scan_height_px = (float)scan_area_size_px.height();

                    float step_size_mm = (float)ui->stepsize_xy->value();
                    float scan_width_mm = (float)scan_area_size.width();
                    float scan_height_mm = (float)scan_area_size.height();

                    file.write(reinterpret_cast<const char*>(&step_size_px), sizeof(step_size_px));
                    file.write(reinterpret_cast<const char*>(&scan_width_px), sizeof(scan_width_px ));
                    file.write(reinterpret_cast<const char*>(&scan_height_px), sizeof(scan_height_px));
                    file.write(reinterpret_cast<const char*>(&step_size_mm), sizeof(step_size_mm));
                    file.write(reinterpret_cast<const char*>(&scan_width_mm), sizeof(scan_width_mm));
                    file.write(reinterpret_cast<const char*>(&scan_height_mm), sizeof(scan_height_mm));

                    file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
                    file.write(reinterpret_cast<const char*>(&x_max), sizeof(x_max));
                    file.write(reinterpret_cast<const char*>(&y_max), sizeof(y_max));
                }
                else
                {
                    qDebug() << "File exists. No overwrite allowed!";
                }

                qDebug() << "DATA SAVED TO FILE. SIZE OF THE VECTOR FOR Y COMP: " << data_tensor.size();
                if(file.is_open())
                {
                    for(auto &v : data_tensor)
                    {
                        for(auto &v1 : v)
                            file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
                    }
                }
                file.close();
                save_x = 0;
                save_y = 0;
                data_tensor.clear();
                temp2d.clear();
                _socket_robot->write("axis_switch=1\n");
            }
            else
            {
                current_scan_point_x = -1;
                QString path = current_scan_datapath+"x_comp_scan_data_tensor.bin";
                std::ofstream file;

                file.open(path.toLocal8Bit(), std::ios::binary);

                if(file)
                {
                    qDebug() << "File does not exists. Created file!";
                    float sp = (float)sweep_points;
                    float x_max = (float)scan_columns;
                    float y_max = (float)scan_rows;

                    float step_size_px = (float)scan_area_size_px.width()/scan_rows;
                    float scan_width_px = (float)scan_area_size_px.width();
                    float scan_height_px = (float)scan_area_size_px.height();

                    float step_size_mm = (float)ui->stepsize_xy->value();
                    float scan_width_mm = (float)scan_area_size.width();
                    float scan_height_mm = (float)scan_area_size.height();

                    file.write(reinterpret_cast<const char*>(&step_size_px), sizeof(step_size_px));
                    file.write(reinterpret_cast<const char*>(&scan_width_px), sizeof(scan_width_px ));
                    file.write(reinterpret_cast<const char*>(&scan_height_px), sizeof(scan_height_px));
                    file.write(reinterpret_cast<const char*>(&step_size_mm), sizeof(step_size_mm));
                    file.write(reinterpret_cast<const char*>(&scan_width_mm), sizeof(scan_width_mm));
                    file.write(reinterpret_cast<const char*>(&scan_height_mm), sizeof(scan_height_mm));

                    file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
                    file.write(reinterpret_cast<const char*>(&x_max), sizeof(x_max));
                    file.write(reinterpret_cast<const char*>(&y_max), sizeof(y_max));
                }
                else
                {
                    qDebug() << "File exists. No overwrite allowed!";
                }

                qDebug() << "DATA SAVED TO FILE. SIZE OF THE VECTOR FOR X COMP: " << data_tensor.size();
                if(file.is_open())
                {
                    for(auto &v : data_tensor)
                    {
                        for(auto &v1 : v)
                            file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
                    }
                }
                file.close();
                save_x = 0;
                save_y = 0;
                instrument_thread_init();
                emit scan_finished_to_wizard();
                emit allow_emit_pos(false);
            }
            break;
        }
        case 4:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Height measure started...");
            emit stop_displaying_point();
            break;
        case 5:
        {
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Height measure done!");

            char value_to_cvt[10];

            for(size_t i=3; i<strlen(msg); i++)
                value_to_cvt[i-3] = msg.at(i);
            float height = 0.0;
            height = atof(value_to_cvt);
            height = roundf(height);
            real_height = int(sqrt(pow(height,2)));
            emit height_measured();
            break;
        }
        case 6:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Robot position");
            break;
        case 7:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Going to the PCB's corner...");
            resetCamera_button_clicked();
            break;
        case 8:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Reached the PCB's corner!");
            ask_robot_for_cam_height();
            break;
        case 9:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Moving to the homeposition...");
            break;
        case 10:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Reached the homeposition!");
            scan_point = 0;
            break;
        case 11:
            ui->robotTerminal->setText("");
            ui->robotTerminal->setText("Scan aborted!");
            instrument_thread_init();
            save_x = 0;
            save_y = 0;
            current_scan_point_x = 0;
            emit allow_emit_pos(false);
            break;
        case 15:
        {
            char value_to_cvt[10];
            for(size_t i=3; i<strlen(msg); i++)
                value_to_cvt[i-3] = msg.at(i);
            float height = 0.0;
            height = atof(value_to_cvt);
            height = roundf(height);
            camera_distance_2 = height;

            if(run_scan_cam_h)
            {
                int w_px = (pcb_size.width()*focal_lenght*1280)/(camera_distance_2*sensor_width);
                int h_px = (pcb_size.height()*focal_lenght*960)/(camera_distance_2*sensor_height);

                emit send_area_to_videothread(w_px*h_px);

                run_scan_cam_h = false;
            }

            qDebug() << "Camera distance: " << camera_distance_2;
            break;
        }
        case 17:
        {
            qDebug() << "Probe rotated!";
            y_comp=false;
            _socket_robot->write("Mes=1\n");
        }
        default:
            ui->robotTerminal->setText("Waiting...");
            break;
    }

}

void scanner_gui::send_robot_coordinates(bool middle)
{
    QString msg = "";
    uint16_t x = 0;
    uint16_t y = 0;

    if(middle)
    {
        x = scan_height_point.x()-10;
        y = scan_height_point.y()-3;

        msg = "fast_x = %1\n";
        msg = msg.arg(QString::number(x));
        _socket_robot->write(msg.toLocal8Bit());
        msg = "fast_y = %1\n";
        msg = msg.arg(QString::number(y));
        _socket_robot->write(msg.toLocal8Bit());
    }
    else if(!middle)
    {
        x = pcb_corner.x() - scan_area_corner.x()-12;
        y = pcb_corner.y() - scan_area_corner.y()-12;

        msg = "x_mes = %1\n";
        msg = msg.arg(QString::number(x));
        _socket_robot->write(msg.toLocal8Bit());
        msg = "y_mes = %1\n";
        msg = msg.arg(QString::number(y));
        _socket_robot->write(msg.toLocal8Bit());
    }
}

void scanner_gui::send_to_top_pcb_edge()
{
    QString msg = "";
    uint16_t x = 0;
    uint16_t y = 0;

    x = pcb_size.width()/2;
    y = pcb_size.height()/2;

    int h = sqrt(pow(x,2)+pow(y,2));

    msg = "zShift = %1\n";
    msg = msg.arg(QString::number(h));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "yShift = %1\n";
    msg = msg.arg(QString::number(-30));
    _socket_robot->write(msg.toLocal8Bit());

    run_scan_cam_h = true;
}

void scanner_gui::ask_robot_for_cam_height()
{
    ask_for_cam_h();
}

void scanner_gui::ask_for_cam_h()
{
    _socket_robot->write("cam_h = 1\n");
}

void scanner_gui::set_scan_step_sizes()
{
    QString msg = "";
    msg = "mes_row_max = %1\n";
    msg = msg.arg(QString::number(scan_area_size.height()/ui->stepsize_xy->value()));
    scan_rows = scan_area_size.height()/ui->stepsize_xy->value();
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_column_max = %1\n";
    msg = msg.arg(QString::number(scan_area_size.width()/ui->stepsize_xy->value()));
    scan_columns = scan_area_size.width()/ui->stepsize_xy->value();
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_row_res = %1\n";
    msg = msg.arg(QString::number(ui->stepsize_xy->value()));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_column_res = %1\n";
    msg = msg.arg(QString::number(ui->stepsize_xy->value()));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_delay = 1.5\n";
    _socket_robot->write(msg.toLocal8Bit());

    qDebug() << "X_steps: " << scan_columns;
    qDebug() << "Y_steps: " << scan_rows;
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

void scanner_gui::on_Tool_Tab_Closed(QVector<Tool*> rTools){
    ui->Probe_dropdown->clear();
    Tools = rTools;
    for (int i = 0; i < Tools.length(); i++) {
        ui->Probe_dropdown->addItem(Tools[i]->tool_name);
        ///qDebug() << Tools[i]->tool_name;
    }
}

void scanner_gui::robotBytesWritten(qint64 b)
{
    Q_UNUSED(b);
}
