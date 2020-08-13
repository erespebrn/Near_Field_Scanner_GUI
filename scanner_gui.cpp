#include "scanner_gui.h"
#include "ui_scanner_gui.h"
#include "scan_settings.h"
#include "imagesettings.h"
#include <QMediaService>
#include <QDebug>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>
#include <QMessageBox>
#include <QPalette>
#include <QtWidgets>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHostAddress>

char Shift_string[80];
QByteArray array;
Q_DECLARE_METATYPE(QCameraInfo)
scanner_gui::scanner_gui() : ui(new Ui::scanner_gui), _socket_robot(this)
{
    ui->setupUi(this);
    //sockets:
    _socket_robot.connectToHost(QHostAddress("192.168.11.2"), 23);
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

    //Camera
    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : availableCameras)
    {
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
        if (cameraInfo == QCameraInfo::defaultCamera())
            videoDeviceAction->setChecked(true);

        ui->menuDevices->addAction(videoDeviceAction);
    }

    setCamera(QCameraInfo::defaultCamera());

    //Default image capture settings
    m_imageSettings.setCodec("jpg");
    m_imageSettings.setResolution(4208, 3120);
    m_imageCapture->setEncodingSettings(m_imageSettings);


    connect(ui->actionImage_Settings, &QAction::triggered, this, &scanner_gui::configureImageSettings);
    connect(videoDevicesGroup, &QActionGroup::triggered, this, &scanner_gui::updateCameraDevice);


    //Mouse events signals
    connect(ui->lastImagePreviewLabel, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));
    connect(ui->lastImagePreviewLabel, SIGNAL(sendQrect(QRect&)), this, SLOT(displayCroppedImage(QRect&)));
}

scanner_gui::~scanner_gui()
{
    delete ui;
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
    ImageSettings settingsDialog(m_imageCapture.data(), &m_imageSettings);
    settingsDialog.setWindowFlags(settingsDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    settingsDialog.exec();
    m_imageCapture->setEncodingSettings(m_imageSettings);
}

void scanner_gui::on_Take_img_button_clicked()
{
    m_camera->searchAndLock();
    takeImage();
    displayCapturedImage();
    m_camera->unlock();
}

void scanner_gui::takeImage()
{
    m_isCapturingImage = true;
    m_imageCapture->capture();
}

void scanner_gui::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void scanner_gui::processCapturedImage(int requestId, const QImage &img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    displayCapturedImage();
}

void scanner_gui::displayCroppedImage(QRect &rect)
{
    const QPixmap* pixmap = ui->lastImagePreviewLabel->pixmap();
    QImage image( pixmap->toImage() );
    QImage cropped = image.copy(rect);
    QImage scaledImage = cropped.scaled(ui->viewfinder->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));
    scaledImage.save( QDir::toNativeSeparators(QDir::homePath() + "/Pictures/cropped_image.PNG"), "PNG",100);

    // *** Determine the real size of the object on an image *** //
    //Since the image taken is scaled, scale factor must be used
    float scale_factor = (float)m_imageSettings.resolution().height()/(float)ui->viewfinder->height();

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float height_cropped = (camera_distance*rect.height()*sensor_height/(focal_lenght*m_imageSettings.resolution().height()))*scale_factor;
    float width_cropped = (camera_distance*rect.width()*sensor_width/(focal_lenght*m_imageSettings.resolution().width()))*scale_factor;

    //Since the used camera image sensor's aspect ratio is 4:3, when using 16:9 ratio the image is cropped.
    //Therefore the special factor (calculated experimentally) added when aspect ratio is 16:9
    if(((float)m_imageSettings.resolution().width()/(float)m_imageSettings.resolution().height()) > 1.5)
        height_cropped = height_cropped*0.73;

    ui->cropped_size->setText("x: "+ QString::number((uint16_t)width_cropped) +"mm" + ",y: " + QString::number((uint16_t)height_cropped) + "mm" );
    ui->cropped_size_px->setText("x: "+ QString::number(rect.width()) +"px" + ",y: " + QString::number(rect.height()) + "px" );
}

void scanner_gui::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void scanner_gui::on_actionReset_Camera_triggered()
{
    displayViewfinder();
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
    connect(m_mediaRecorder.data(), QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
            this, &scanner_gui::displayRecorderError);

    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

    //connect(ui->exposureCompensation, &QAbstractSlider::valueChanged, this, &scanner_gui::setExposureCompensation);

    m_camera->setViewfinder(ui->viewfinder);

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
            takeImage();
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
}

void scanner_gui::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
}

void scanner_gui::updateCameraDevice(QAction *action)
{
    setCamera(qvariant_cast<QCameraInfo>(action->data()));
}

/*void scanner_gui::readyForCapture(bool ready)
{
    ui->Take_img_button->setEnabled(ready);
}*/

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
    ui->mouse_position_label->setText("x: "+ QString::number(pos.x()) + ",y: " + QString::number(pos.y()));
}



void scanner_gui::on_scan_settings_button_clicked()
{
    scan_settings scan_settings(this);
    scan_settings.setModal(true);
    scan_settings.exec();

}





