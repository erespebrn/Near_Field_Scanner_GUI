#include "scanner_gui.h"
#include "scan_settings.h"
#include "ui_scanner_gui.h"


#include <cmath>
#include <QMediaService>
#include <QDebug>
#include <QTimer>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>
#include <QMessageBox>
#include <QPalette>
#include <QtWidgets>
#include <QLabel>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QHostAddress>
#include <QPainter>

char Shift_string[80];
QByteArray array;

Q_DECLARE_METATYPE(QCameraInfo)
scanner_gui::scanner_gui() : ui(new Ui::scanner_gui), _socket_robot(this)
{
    ui->setupUi(this);

    robot_init();
    video_thread_init();
    instrument_thread_init();

    //Mouse events signals
    connect(ui->lastImagePreviewLabel, SIGNAL(sendQrect(QRect&)), this, SLOT(displayCroppedImage(QRect&)));

    // Delete last scan settings file
    QFile file(QCoreApplication::applicationDirPath() + "/scansettings.ini");
    if(file.exists())
        file.remove();

    // Minor init settings
    QSizePolicy sp_croppedsize = ui->cropped_size->sizePolicy();
    sp_croppedsize.setRetainSizeWhenHidden(true);
    ui->cropped_size->setSizePolicy(sp_croppedsize);
    sp_croppedsize = ui->cropped_size_px->sizePolicy();
    sp_croppedsize.setRetainSizeWhenHidden(true);
    ui->cropped_size_px->setSizePolicy(sp_croppedsize);
    ui->cropped_size->setVisible(false);
    ui->cropped_size_px->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);

}

scanner_gui::~scanner_gui()
{
    _socket_robot.write("demo = 0");
    _socket_robot.waitForBytesWritten();
    _socket_robot.disconnect();
    delete ui;
}

void scanner_gui::on_robot_connect_button_clicked()
{
    robot_init();
}

void scanner_gui::robot_init()
{
    char robot_msg[128];
    QString send_msg = "";

    // *** Robot TCP connection *** //
    _socket_robot.connectToHost(QHostAddress(robot_ip_address), 23);
    if(_socket_robot.waitForConnected(1))
    {
       ui->robot_connect_button->setEnabled(false);
       ui->robot_connect_button->setText("Connected");
       ui->robotManualControl_frame->setEnabled(true);
       _socket_robot.write(send_msg.toLocal8Bit());
       _socket_robot.waitForReadyRead(20);
       _socket_robot.read(robot_msg,16);
       send_msg = "as\n";
       _socket_robot.write(send_msg.toLocal8Bit());
       _socket_robot.waitForReadyRead(20);
       send_msg = "EXECUTE main\n";
       _socket_robot.write(send_msg.toLocal8Bit());
       _socket_robot.waitForReadyRead(20);

       _socket_robot.read(robot_msg,128);
       qDebug() << robot_msg;
       ui->robotTerminal->setText(QString(robot_msg));
       // *** //
    }
    else
    {
        QMessageBox::warning(this, "Robot error", "Robot not connected!");
        ui->robotManualControl_frame->setEnabled(false);
    }
}

void scanner_gui::video_thread_init()
{
    QThread* thread1 = new QThread;
    VideoThread* videothread = new VideoThread;
    videothread->moveToThread(thread1);
    connect(thread1, SIGNAL(started()), videothread, SLOT(start()));
    connect(thread1, SIGNAL(finished()), thread1, SLOT(deleteLater()));
    connect(videothread, SIGNAL(finished()), thread1, SLOT(quit()));
    connect(videothread, SIGNAL(finished()), videothread, SLOT(deleteLater()));
    connect(videothread, SIGNAL(readyImg(QImage, int, int)), this, SLOT(cv_getframe(QImage, int, int)));
    connect(videothread, SIGNAL(error(QString)), this, SLOT(cameraError(QString)));
    connect(videothread, SIGNAL(cameraOpened()), this, SLOT(cameraConnected()));
    thread1->start();
}

void scanner_gui::instrument_thread_init()
{
    QThread * thread2 = new QThread;
    Instrument_Thread * insthread = new Instrument_Thread(sa_ip_address, vna_ip_address);
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

void scanner_gui::on_Take_img_button_clicked()
{
    ui->cropped_size->setVisible(false);
    ui->cropped_size_px->setVisible(false);
    ui->Take_img_button->setEnabled(false);
    ui->Take_img_button->setText("Picture taken");
    displayCapturedImage();
    processCapturedImage(lastImage);
}

void scanner_gui::on_resetCamera_button_clicked()
{
    displayViewfinder();
    ui->Take_img_button->setEnabled(true);
    ui->Take_img_button->setText("Take Picture");
    ui->cropped_size->setVisible(false);
    ui->cropped_size_px->setVisible(false);
}

void scanner_gui::cameraConnected()
{
    ui->camera_connect_button->setEnabled(false);
    ui->camera_connect_button->setText("Connected");
}

void scanner_gui::cameraError(QString error)
{
    ui->camera_connect_button->setEnabled(true);
    ui->camera_connect_button->setText("Connect");
    ui->liveStream->setText("Camera error. Try to reconect!");
    QMessageBox::critical(this, "Camera error", error);
}

void scanner_gui::on_camera_connect_button_clicked()
{
    video_thread_init();
}

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

void scanner_gui::cv_getframe(QImage frame, int o_x, int o_y)
{
    QImage scaledframe_cv = frame.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    lastImage = scaledframe_cv;
    cv_robot_origin = cv::Point(o_x, o_y);
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
    ui->cropped_size->setVisible(true);
    ui->cropped_size_px->setVisible(true);
    croppedOrigin = rect;
    const QPixmap* pixmap = ui->lastImagePreviewLabel->pixmap();
    QImage image( pixmap->toImage() );
    QImage cropped = image.copy(rect);
    QImage scaledImage = cropped.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    scaledImage.save(QDir::toNativeSeparators(QDir::homePath() + "/Pictures/cropped_image.PNG"), "PNG",100);

    // *** Determine the real size of the object on an image *** //
    //Since the image taken is scaled, scale factor must be used
    float scale_factor = (float)resolution_max_height/(float)ui->lastImagePreviewLabel->height();

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float height_cropped = (camera_distance*rect.height()*sensor_height/(focal_lenght*(float)resolution_max_height))*scale_factor;
    float width_cropped = (camera_distance*rect.width()*sensor_width/(focal_lenght*(float)resolution_max_width))*scale_factor;


    float x_dist_px = croppedOrigin.x() - cv_robot_origin.x;
    float y_dist_px = croppedOrigin.y() - cv_robot_origin.y;

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*(float)resolution_max_width))*scale_factor;
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*(float)resolution_max_height))*scale_factor;

    float distance = sqrt(pow(x_dist_mm,2) + pow(y_dist_mm,2));

    ui->cropped_size->setText("x: "+ QString::number((uint16_t)width_cropped) +"mm" + ", y: " + QString::number((uint16_t)height_cropped) + "mm" );
    ui->cropped_size_px->setText("x: "+ QString::number((uint16_t)x_dist_mm) +"mm" + ", y: " + QString::number((uint16_t)y_dist_mm) + "mm \n Lenght: " + QString::number((uint16_t)distance) + "mm");
}

void scanner_gui::on_scan_settings_button_clicked()
{
    if(vna_connected_bool || sa_connected_bool)
    {
        QString msg = "";
        bool wo_vna = false;
        bool wo_sa = false;

        if(sa_connected_bool)
        {
            emit insthread_stop();
            _socket_vna.connectToHost(vna_ip_address, 5025);
            _socket_vna.waitForConnected(10);

            if(_socket_vna.state() == QAbstractSocket::ConnectedState)
            {
                msg = "*RST\n";
                _socket_vna.write(msg.toLocal8Bit());
                _socket_vna.waitForBytesWritten();
                msg = "SYST:DISP:UPD ON\n";
                _socket_vna.write(msg.toLocal8Bit());
                _socket_vna.waitForBytesWritten();
            }
        }
        else
        {
            QMessageBox::StandardButton reply = QMessageBox::warning(this, "Spectrum Analyzer", "SA not connected. Continue?", QMessageBox::Yes | QMessageBox::No);

            if(reply == QMessageBox::Yes)
                wo_sa = true;
        }

        if(wo_sa)
        {
            if(vna_connected_bool)
            {
                emit insthread_stop();
                _socket_vna.connectToHost(vna_ip_address, 5025);
                _socket_vna.waitForConnected(10);

                if(_socket_vna.state() == QAbstractSocket::ConnectedState)
                {
                    msg = "*RST\n";
                    _socket_vna.write(msg.toLocal8Bit());
                    _socket_vna.waitForBytesWritten();
                    msg = "SYST:DISP:UPD ON\n";
                    _socket_vna.write(msg.toLocal8Bit());
                    _socket_vna.waitForBytesWritten();
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
                scan_settings scan_settings(&_socket_sa, &_socket_vna, this);
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();
            }
            else if(wo_vna)
            {
                qDebug("No vna");
                scan_settings scan_settings(&_socket_sa, false, this);
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();

            }
            else if(wo_sa)
            {
                qDebug("No sa");
                scan_settings scan_settings(&_socket_vna, true, this);
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();
            }

            instrument_thread_init();
        }
    }
    else
    {
        QMessageBox::critical(this, "No instrument", "No measurement instrument connected!");
    }
}

void scanner_gui::on_scan_height_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    QString mystring = "measuring_height = %1";
    mystring = mystring.arg(QString::number(ui->scan_height->value()));
    _socket_robot.write("measuring_height = 1");
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
    _socket_robot.waitForBytesWritten(30);
}

void scanner_gui::on_measure_height_clicked()
{
    _socket_robot.write("mesheight = 1");
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
    _socket_robot.waitForBytesWritten(30);
}

void scanner_gui::on_stepsize_x_valueChanged(double arg1)//stepsize x
{
    Q_UNUSED(arg1);
    QString mystring = "dist = %1";
    mystring = mystring.arg(QString::number(ui->stepsize_x->value()));
    _socket_robot.write(mystring.toLocal8Bit());       //if it doesn't work, try toUtf8()
    _socket_robot.waitForBytesWritten(30);
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_stepsize_y_valueChanged(double arg1)//stepsize y
{
    Q_UNUSED(arg1);
    QString mystring = "dist = %1";
    mystring = mystring.arg(QString::number(ui->stepsize_y->value()));
    _socket_robot.write(mystring.toLocal8Bit());       //if it doesn't work, try toUtf8()
    _socket_robot.waitForBytesWritten(30);
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_Start_scan_button_clicked()
{
    _socket_robot.write("demo = 1");
    _socket_robot.waitForReadyRead();
    char test[128];
    _socket_robot.read(test, 128);
    qDebug() << test;
    _socket_robot.write(QByteArray("\n"));
    _socket_robot.waitForBytesWritten(30);
    QString mystring = "dist = %1";
    mystring = mystring.arg(QString::number(ui->stepsize_x->value()));
    _socket_robot.write(mystring.toUtf8());       //if it doesn't work, try toUtf8()
    _socket_robot.waitForBytesWritten(30);
    _socket_robot.write(QByteArray("\n"));
    //Start scan button
}

void scanner_gui::on_stop_scan_button_clicked()
{
    _socket_robot.write("demo = 0");
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
    _socket_robot.waitForBytesWritten(30);
}

void scanner_gui::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void scanner_gui::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void scanner_gui::on_Y_plus_button_pressed()
{
    double stepsize_y = ui->stepsize_y->value();
    sprintf(Shift_string, "yShift = %lf", -stepsize_y);
    _socket_robot.write(QByteArray(Shift_string));
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_Y_minus_button_pressed()
{
    double stepsize_y = ui->stepsize_y->value();
    sprintf(Shift_string, "yShift = %lf", stepsize_y);
    _socket_robot.write(QByteArray(Shift_string));
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_X_plus_button_pressed()
{
    double stepsize_x = ui->stepsize_x->value();
    sprintf(Shift_string, "xShift = %lf", -stepsize_x);
    _socket_robot.write(QByteArray(Shift_string));
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_X_minus_button_pressed()
{
    double stepsize_x = ui->stepsize_x->value();
    sprintf(Shift_string, "xShift = %lf", stepsize_x);
    _socket_robot.write(QByteArray(Shift_string));
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_Z_plus_pressed()
{
    double stepsize_z = ui->stepsize_x->value();
    sprintf(Shift_string, "zShift = %lf", stepsize_z);
    _socket_robot.write(QByteArray(Shift_string));
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_Z_minus_pressed()
{
    double stepsize_z = ui->stepsize_x->value();
    sprintf(Shift_string, "zShift = %lf", -stepsize_z);
    _socket_robot.write(QByteArray(Shift_string));
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
}

void scanner_gui::on_home_button_clicked()
{
    _socket_robot.write("takepic = 1");
    _socket_robot.waitForReadyRead(150);
    _socket_robot.write(QByteArray("\n"));
    _socket_robot.waitForReadyRead(50);
    _socket_robot.waitForReadyRead(50);
    array = _socket_robot.readAll();
    qDebug() << array;
}
