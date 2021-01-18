#ifndef SCANWIZARD_H
#define SCANWIZARD_H

#include <QWidget>
#include <QTimer>
#include "robot.h"

namespace Ui {
class ScanWizard;
}

class ScanWizard : public QWidget
{
    Q_OBJECT

public:
    explicit ScanWizard(QWidget *parent = nullptr);
    ~ScanWizard();
signals:
    void detect_pcb(bool);
    void send_robot_to_origin(bool);
    void scan_area_origin_detect(bool);
    void set_scan_settings(int);
    void run_scan(bool);
    void measure_height();
    void send_for_2nd_takepic();
    void ask_for_cam_height();
    void allow_emit_pos(bool);

private slots:
    void on_Next_button_clicked();
    void pcb_found();
    void check_for_pcb();
    void take_coord(QPoint, QRect);
    void on_Cancel_button_clicked();
    void height_measure_finished();
    void scan_finished();
    void check_for_instruments_created();
    void inst_created();

private:
    Ui::ScanWizard *ui;
    QTimer * timer;

    bool _pcb_found = false;
    bool _corner_found = false;
    bool ins_creat = false;
    Robot * robot;
    QPoint pcb_corner;
    QRect pcb_size;
    uint8_t step = 0;
    uint8_t tries = 0;
    QWidget * par;

    bool area_selected = false;
};

#endif // SCANWIZARD_H
