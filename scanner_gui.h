#ifndef SCANNER_GUI_H
#define SCANNER_GUI_H
#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QScopedPointer>
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRubberBand>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class scanner_gui; }
QT_END_NAMESPACE

class scanner_gui : public QMainWindow
{
    Q_OBJECT

public:
    scanner_gui();
    ~scanner_gui();

private slots:
    void on_Start_scan_button_clicked();
    void on_scan_height_valueChanged(double arg1);
    void on_stepsize_x_valueChanged(double arg1);
    void on_stepsize_y_valueChanged(double arg1);
    void on_Take_img_button_clicked();

    //Robot manual control buttons
    void on_Y_plus_button_pressed();
    void on_Y_minus_button_pressed();
    void on_X_plus_button_pressed();
    void on_X_minus_button_pressed();
    void on_Z_plus_pressed();
    void on_Z_minus_pressed();

    void setCamera(const QCameraInfo &cameraInfo);

    void startCamera();
    void stopCamera();

    void record();
    void pause();
    void stop();
    void setMuted(bool);

    void toggleLock();
    void takeImage();
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void configureCaptureSettings();
    //void configureVideoSettings();
    void configureImageSettings();

    void displayRecorderError();
    void displayCameraError();

    void updateCameraDevice(QAction *action);

    //void updateCameraState(QCamera::State);
    //void updateCaptureMode();
    //void updateRecorderState(QMediaRecorder::State state);
    void setExposureCompensation(int index);

    void updateRecordTime();

    void processCapturedImage(int requestId, const QImage &img);
    //void updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason);

    void displayViewfinder();
    void displayCapturedImage();
    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

    void on_scan_settings_button_clicked();

    void on_home_button_clicked();
    void on_actionReset_Camera_triggered();
    void on_measure_height_clicked();
    void on_stop_scan_button_clicked();

    void displayCroppedImage(QRect& rect);
    void showMousePosition(QPoint& pos);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::scanner_gui *ui;

    QScopedPointer<QCamera> m_camera;
    QScopedPointer<QCameraImageCapture> m_imageCapture;
    QScopedPointer<QMediaRecorder> m_mediaRecorder;
    QTcpSocket _socket_robot;
    QTcpSocket _socket_sa;

    QImageEncoderSettings m_imageSettings;
    QAudioEncoderSettings m_audioSettings;
    QVideoEncoderSettings m_videoSettings;

    QString m_videoContainerFormat;
    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
};

#endif // SCANNER_GUI_H
