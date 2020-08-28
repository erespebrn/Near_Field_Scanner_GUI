#include "scanner_gui.h"
#include "scan_settings.h"
#include "ui_scanner_gui.h"
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

    camera_init();

    // *** Robot TCP connection *** //
    _socket_robot.connectToHost(QHostAddress(robot_ip_address), 23);
    _socket_robot.write("");
    _socket_robot.waitForReadyRead(20);
    _socket_robot.waitForReadyRead(20);
    _socket_robot.write("a");
    _socket_robot.write("s");
    _socket_robot.waitForReadyRead(20);
    _socket_robot.write("\r\n");
    //initialize robot:
    _socket_robot.waitForReadyRead(20);
    _socket_robot.waitForReadyRead(20);
    _socket_robot.write("EXECUTE main");
    _socket_robot.waitForReadyRead(20);
    _socket_robot.write("\n");
    // *** //

    connect(&_socket_sa, &QAbstractSocket::connected, this, &scanner_gui::sa_connected);
    connect(&_socket_sa, &QAbstractSocket::disconnected, this, &scanner_gui::sa_disconnected);

    //Mouse events signals
    connect(ui->lastImagePreviewLabel, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));
    connect(ui->lastImagePreviewLabel, SIGNAL(sendQrect(QRect&)), this, SLOT(displayCroppedImage(QRect&)));

    QFile file(QCoreApplication::applicationDirPath() + "/scansettings.ini");
    if(file.exists())
        file.remove();

    QSizePolicy sp_croppedsize = ui->cropped_size->sizePolicy();
    sp_croppedsize.setRetainSizeWhenHidden(true);
    ui->cropped_size->setSizePolicy(sp_croppedsize);
    sp_croppedsize = ui->cropped_size_px->sizePolicy();
    sp_croppedsize.setRetainSizeWhenHidden(true);
    ui->cropped_size_px->setSizePolicy(sp_croppedsize);
    ui->cropped_size->setVisible(false);
    ui->cropped_size_px->setVisible(false);
}

scanner_gui::~scanner_gui()
{
    delete ui;
}

void scanner_gui::camera_init()
{
    if(cv_camera.open(0, cv::CAP_ANY))
    {
        ui->camera_connect_button->setEnabled(false);
        ui->camera_connect_button->setText("Connected");
    }
    cv_camera.set(3, 640);
    cv_camera.set(4, 480);
    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &scanner_gui::cv_getframe);
    timer->start(1);
    ui->lastImagePreviewLabel->setAlignment(Qt::AlignLeft);
    ui->liveStream->setAlignment(Qt::AlignCenter);
    ui->stackedWidget->setCurrentIndex(0);
}

void scanner_gui::on_Take_img_button_clicked()
{
    ui->cropped_size->setVisible(false);
    ui->cropped_size_px->setVisible(false);
    timer->stop();
    displayCapturedImage();
    processCapturedImage(0,lastImage);
}

void scanner_gui::cv_getframe()
{
    cv::Mat frame_cv;
    cv::Mat frame_gray;
    cv::Mat frame_blur;
    cv::Mat frame_canny;

    cv_camera.read(frame_cv);
    cv::cvtColor(frame_cv, frame_gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(frame_gray, frame_blur,cv::Size(7,7),1);
    cv::Canny(frame_blur,frame_canny,100,100);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(frame_canny(cv::Rect(0,0,50,50)), contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if(!contours.empty())
    {
        cv::circle(frame_cv, contours[0][0], 3, cv::Scalar(255,0,0),1);
        cv::putText(frame_cv, "(0,0)", cv::Point(contours[0][0].x+5, contours[0][0].y-5), cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255,0,0),1);
    }
    QImage frame_qt = MatToQImage(frame_cv);
    QImage scaledframe_qt = frame_qt.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    lastImage = frame_qt;
    robot_origin = contours[0][0];
    ui->liveStream->setPixmap(QPixmap::fromImage(scaledframe_qt));
}


void scanner_gui::processCapturedImage(int requestId, const QImage &img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->lastImagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    displayCapturedImage();
}

void scanner_gui::displayCroppedImage(QRect &rect)
{
    ui->cropped_size->setVisible(true);
    ui->cropped_size_px->setVisible(true);

    const QPixmap* pixmap = ui->lastImagePreviewLabel->pixmap();
    QImage image( pixmap->toImage() );
    QImage cropped = image.copy(rect);
    QImage scaledImage = cropped.scaled(ui->liveStream->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    scaledImage.save(QDir::toNativeSeparators(QDir::homePath() + "/Pictures/cropped_image.PNG"), "PNG",100);

    // *** Determine the real size of the object on an image *** //
    //Since the image taken is scaled, scale factor must be used
    float scale_factor = 3120.0/(float)ui->lastImagePreviewLabel->height();

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float height_cropped = (camera_distance*rect.height()*sensor_height/(focal_lenght*3120))*scale_factor;
    float width_cropped = (camera_distance*rect.width()*sensor_width/(focal_lenght*4208))*scale_factor;

    //The used camera image sensor's aspect ratio is 4:3

    ui->cropped_size->setText("x: "+ QString::number((uint16_t)width_cropped) +"mm" + ",y: " + QString::number((uint16_t)height_cropped) + "mm" );
    ui->cropped_size_px->setText("x: "+ QString::number(rect.width()) +"px" + ",y: " + QString::number(rect.height()) + "px" );
}

/* Here we see the valueboxes. We use the "on value change" because its what is under the "doubleSpinBox" however -
  maybe you can use the functions from the other box types???? */
void scanner_gui::on_scan_height_valueChanged(double arg1)//scan height
{
    //when the value is changed, you get arg1 here
}

void scanner_gui::on_measure_height_clicked()
{

    _socket_robot.write("mesheight = 1");
    _socket_robot.waitForReadyRead();
    _socket_robot.write(QByteArray("\n"));
    _socket_robot.waitForBytesWritten(30);



//    QString mystring = "mesheight";
//    mystring = mystring.arg(QString::number(ui->scan_height->value()));
//    _socket_robot.write(mystring.toLocal8Bit());       //if it doesn't work, try toUtf8()
//    _socket_robot.waitForBytesWritten(30);
//    _socket_robot.write(QByteArray("\n"));

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
    //when the value is changed, you get arg1 here
}

void scanner_gui::on_Start_scan_button_clicked()
{
    _socket_robot.write("demo = 1");
    _socket_robot.waitForReadyRead();
    char test[128];
    _socket_robot.read(test, 128);
    //qDebug() << test;
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

void scanner_gui::configureImageSettings()
{

}

void scanner_gui::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void scanner_gui::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void scanner_gui::on_resetCamera_button_clicked()
{
    displayViewfinder();
    timer->start(1);
    ui->cropped_size->setVisible(false);
    ui->cropped_size_px->setVisible(false);
}

void scanner_gui::on_Y_plus_button_pressed()
{
    //can use QByteArray x("whatever")
    //can use x.append("whatever2"), to append.
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

void scanner_gui::setCamera(const QCameraInfo &cameraInfo)
{
    m_camera.reset(new QCamera(cameraInfo));

    //connect(m_camera.data(), &QCamera::stateChanged, this, &scanner_gui::updateCameraState);
    connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), this, &scanner_gui::displayCameraError);

    m_mediaRecorder.reset(new QMediaRecorder(m_camera.data()));
    //connect(m_mediaRecorder.data(), &QMediaRecorder::stateChanged, this, &scanner_gui::updateRecorderState);

    m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));

    connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, &scanner_gui::updateRecordTime);
    connect(m_mediaRecorder.data(), QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error), this, &scanner_gui::displayRecorderError);

    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Camera")));

    //connect(ui->exposureCompensation, &QAbstractSlider::valueChanged, this, &scanner_gui::setExposureCompensation);


    //updateCameraState(m_camera->state());
    //updateLockStatus(m_camera->lockStatus(), QCamera::UserRequest);
    //updateRecorderState(m_mediaRecorder->state());

    //connect(m_imageCapture.data(), &QCameraImageCapture::readyForCaptureChanged, this, &scanner_gui::readyForCapture);
    connect(m_imageCapture.data(), &QCameraImageCapture::imageCaptured, this, &scanner_gui::processCapturedImage);
    connect(m_imageCapture.data(), &QCameraImageCapture::imageSaved, this, &scanner_gui::imageSaved);
    connect(m_imageCapture.data(), QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
            this, &scanner_gui::displayCaptureError);

    //connect(m_camera.data(), QOverload<QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged),
    //        this, &scanner_gui::updateLockStatus);

    if (m_camera->isCaptureModeSupported(QCamera::CaptureStillImage))
        m_camera->setCaptureMode(QCamera::CaptureStillImage);

    //ui->captureWidget->setTabEnabled(0, (m_camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
    //ui->captureWidget->setTabEnabled(1, (m_camera->isCaptureModeSupported(QCamera::CaptureVideo)));

    //updateCaptureMode();
//    QCameraFocus *focus = m_camera->focus();
//    focus->setFocusPointMode(QCameraFocus::FocusPointAuto);
    m_camera->start();
    displayViewfinder();

    if(m_camera->status() == QCamera::ActiveStatus)
    {
        ui->camera_connect_button->setEnabled(false);
        ui->camera_connect_button->setText("Connected");
    }
}

void scanner_gui::keyPressEvent(QKeyEvent * event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        displayViewfinder();
        m_camera->searchAndLock();
        event->accept();
        break;
    case Qt::Key_Camera:
        if (m_camera->captureMode() == QCamera::CaptureStillImage) {

        } else {
            if (m_mediaRecorder->state() == QMediaRecorder::RecordingState)
                stop();
            else
                record();
        }
        event->accept();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void scanner_gui::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        m_camera->unlock();
        break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
}

void scanner_gui::updateRecordTime()
{
    QString str = QString("Recorded %1 sec").arg(m_mediaRecorder->duration()/1000);
    ui->statusbar->showMessage(str);
}

void scanner_gui::configureCaptureSettings()
{
    switch (m_camera->captureMode()) {
    case QCamera::CaptureStillImage:
        configureImageSettings();
        break;
    default:
        break;
    }
}

void scanner_gui::record()
{
    m_mediaRecorder->record();
    updateRecordTime();
}

void scanner_gui::pause()
{
    m_mediaRecorder->pause();
}

void scanner_gui::stop()
{
    m_mediaRecorder->stop();
}

void scanner_gui::setMuted(bool muted)
{
    m_mediaRecorder->setMuted(muted);
}

void scanner_gui::toggleLock()
{
    switch (m_camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
        m_camera->unlock();
        break;
    case QCamera::Unlocked:
        m_camera->searchAndLock();
    }
}

void scanner_gui::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id)
    Q_UNUSED(error)
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
    ui->camera_connect_button->setEnabled(true);
    ui->camera_connect_button->setText("Connect");
}

void scanner_gui::startCamera()
{
    m_camera->start();
}

void scanner_gui::stopCamera()
{
    m_camera->stop();
}

void scanner_gui::setExposureCompensation(int index)
{
    m_camera->exposure()->setExposureCompensation(index*0.5);
}

void scanner_gui::displayRecorderError()
{
    QMessageBox::warning(this, tr("Capture Error"), m_mediaRecorder->errorString());
    ui->camera_connect_button->setEnabled(true);
    ui->camera_connect_button->setText("Connect");
}

void scanner_gui::displayCameraError()
{
    QMessageBox::warning(this, tr("No Camera Error"), m_camera->errorString());
    ui->camera_connect_button->setEnabled(true);
    ui->camera_connect_button->setText("Connect");
}

void scanner_gui::updateCameraDevice(QAction *action)
{
    setCamera(qvariant_cast<QCameraInfo>(action->data()));
}

void scanner_gui::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id)
    ui->statusbar->showMessage(tr("Captured \"%1\"").arg(QDir::toNativeSeparators(fileName)));

    m_isCapturingImage = false;
    if (m_applicationExiting)
        close();
}

void scanner_gui::closeEvent(QCloseEvent *event)
{
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
}

void scanner_gui::showMousePosition(QPoint &pos)
{

}

void scanner_gui::sa_connected()
{
    ui->sa_connect_btn->setEnabled(false);
    ui->sa_connect_btn->setText("Connected");
}

void scanner_gui::sa_disconnected()
{
    ui->sa_connect_btn->setEnabled(true);
    ui->sa_connect_btn->setText("Connect");
}

void scanner_gui::on_scan_settings_button_clicked()
{
    if(sa_connected_bool)
    {
        scan_settings scan_settings(&_socket_sa, this);
        scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        scan_settings.setModal(true);
        scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
        scan_settings.exec();
    }
    else
    {
        QMessageBox::StandardButton reply = QMessageBox::warning(this, "Not connected", "Try to connect?", QMessageBox::Yes | QMessageBox::No);

        if(reply == QMessageBox::Yes)
        {
            on_sa_connect_btn_clicked();
            if(_socket_sa.state() == QAbstractSocket::ConnectedState)
            {
                scan_settings scan_settings(&_socket_sa, this);
                scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                scan_settings.setModal(true);
                scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
                scan_settings.exec();
            }
        }
    }

}

void scanner_gui::on_sa_connect_btn_clicked()
{
    // *** SIGLENT SSA3032X spectrum analyzer TCP connection *** //
    //Establish a connection with the SSA3032X spectrum analyzer
    _socket_sa.connectToHost(QHostAddress(sa_ip_address), 5024);
    _socket_sa.waitForReadyRead(1);
    //Read the welcome message so it's not in the read buffer any more
    char welcome_msg[128];
    _socket_sa.read(welcome_msg, 128);
    qDebug() << welcome_msg;
    //Check if the connection succeeded
    if(_socket_sa.state() == QAbstractSocket::UnconnectedState)
    {
        sa_connected_bool = false;
        QMessageBox::warning(this, "Connection error!", "Connection to the SSA3032X failed!");
    }
    else
    {
        sa_connected_bool = true;
        //Reset command for the device
        //_socket_sa.write("*RST");
        //_socket_sa.waitForBytesWritten();
        //Set the SA3032X local time and date
        QDate mydate = QDate::currentDate();
        QTime mytime = QTime::currentTime();
        QString time = mytime.toString("hhmmss");
        QString date = mydate.toString("yyyyMMdd");
        QString time_cmd = ":SYSTem:TIME %1\n";
        time_cmd = time_cmd.arg(time);
        QString date_cmd = ":SYSTem:DATE %1\n";
        date_cmd = date_cmd.arg(date);
        _socket_sa.write(date_cmd.toUtf8());
        _socket_sa.waitForBytesWritten();
        _socket_sa.write(time_cmd.toUtf8());
        _socket_sa.waitForBytesWritten();
    }
    // *** //
}

void scanner_gui::on_refresh_connection_btn_clicked()
{
    //Check if the connections are still on line

    //SSA3032X Spectrum Analyzer connection test
    //If the device respond with any character different than 'c', connection still online
    //Else, disconnect the socket and generate the disconnect() signal
    char feedback[128];
    strcpy(feedback, "c");
    _socket_sa.write(":SYSTem:COMMunicate:LAN:IPADdress?");
    _socket_sa.waitForReadyRead(1);
    _socket_sa.read(feedback,128);
    if(feedback[0] == 'c')
    {
        _socket_sa.disconnectFromHost();
    }
    // *** //
}

void scanner_gui::on_camera_connect_button_clicked()
{
    camera_init();
}

QImage scanner_gui::MatToQImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    if(mat.type()==CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    return QImage();
}
