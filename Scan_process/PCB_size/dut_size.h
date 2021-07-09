#ifndef DUT_SIZE_H
#define DUT_SIZE_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QDebug>
#include <math.h>



class DUT_size : public QObject
{
    Q_OBJECT

signals:
    void send_area_to_videothread(qint64);
    void send_coord_to_wizard(QPoint, QRect);

public slots:

    //void cv_getcoord(bool, int, int, int, int, int, int);

    void receive_selected_pcb(QRect &);
    void receive_scanheight_point(QPoint);
    void receive_pcb_corner_point(QPoint p)   { pcb_corner = p; };
    void receive_pcb_ref_point(QPoint);
    void receive_scan_area(QRect&);

public:
    static QPoint scan_pcb_corner;
    static QRect croppedOrigin;
    static const float camera_distance;
    static uint32_t real_height;
    static float camera_distance_2;


    DUT_size();
    ~DUT_size();
    void send_area_request();


    QPointF corner{};
    QPointF scan_height_point{};
    QPointF pcb_corner{};
    QPointF pcb_ref_point{};

    QRect size_mm{};
    QRect size_px{};

private:


    // Camera image sensor dimensions
    const double sensor_width{4.54};
    const double sensor_height{3.42};
    const double focal_lenght{3.81};

    const QPoint robot_origin_fixed{640,480};
};

#endif // DUT_SIZE_H
