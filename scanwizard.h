#ifndef SCANWIZARD_H
#define SCANWIZARD_H

#include <QWidget>
#include <QTimer>
#include "robot.h"
#include "dut_size.h"

namespace Ui {
class ScanWizard;
}

class ScanWizard : public QWidget
{
    Q_OBJECT

public:
    explicit ScanWizard(QWidget *parent = nullptr ,Robot *r = nullptr, DUT_size *p = nullptr, DUT_size *s = nullptr);
    ~ScanWizard();
signals:
    void detect_pcb(bool);
    void scan_area_origin_detect(bool);
    void set_scan_settings(int);
    void save_scanpath();
    void allow_emit_pos(bool);
    void scan_stopped();
    void send_dut_name(QString);
    void kill_cv_window();

private slots:
    void on_Next_button_clicked();
    void pcb_found();
    void check_for_pcb();
    void take_coord(QPoint, QRect);
    void on_Cancel_button_clicked();
    void height_measure_finished();
    void scan_finished();
    void scan_aborted();
    void check_for_instruments_created();
    void inst_created();
    void on_dut_name_textChanged(const QString &arg1);
    void height_meas_point_selected();
    void scan_error();

private:
    Ui::ScanWizard *ui;
    QTimer * timer;

    bool _pcb_found = false;
    bool _corner_found = false;
    bool ins_creat = false;
    Robot *robot;
    DUT_size *pcb;
    DUT_size *scan_area;
    QPoint pcb_corner;
    QRect pcb_size;
    uint8_t step = 0;
    uint8_t tries = 0;
    QWidget *par;
    QString dut_name;

    void est_time_init();
    void update_time();
    int s=0, m=0, h=0, time=0;


    bool area_selected = false;
    bool scan_done = false;
};

#endif // SCANWIZARD_H
