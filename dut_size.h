#ifndef DUT_SIZE_H
#define DUT_SIZE_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <math.h>



class DUT_size : public QObject
{
    Q_OBJECT

signals:
    void send_area_to_videothread(qint64);
    void send_coord_to_wizard(QPoint, QRect);

public slots:
    void receive_cropped_area(QRect&);
    void cv_getcoord(bool, int, int, int, int, int, int);
    void receive_scanheight_point(int, int);

public:
    static QPoint scan_pcb_corner;
    static QRect croppedOrigin;
    static const uint32_t camera_distance = 876;
    static uint32_t real_height;
    static uint32_t camera_distance_2;


    DUT_size();
    void send_area_request();

    QPoint corner;
    QPoint origin;
    QPoint scan_height_point;
    double step_size;
    QRect size_mm;
    QRect size_px;

private:
    // Camera image sensor dimensions
    const float sensor_width = 4.54;
    const float sensor_height = 3.42;
    const float focal_lenght = 3.81;
};

#endif // DUT_SIZE_H
