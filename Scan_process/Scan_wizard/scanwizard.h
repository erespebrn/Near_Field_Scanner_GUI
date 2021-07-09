#ifndef SCANWIZARD_H
#define SCANWIZARD_H

#include <QWidget>
#include <QTimer>

#include "videothread.h"
#include "ui_scanner_gui.h"
#include "TCP_IP/Robot/robot.h"
#include "Scan_process/PCB_size/dut_size.h"
#include "Scan_process/Scan_wizard/scan_interface.h"
#include "Scan_process/Instrument_settings/scan_settings.h"

namespace Ui {
class ScanWizard;
}

class ScanWizard : public Scan_interface
{
    Q_OBJECT
public:
    explicit ScanWizard(QWidget *parent = nullptr, Robot *r = nullptr, VideoThread *v = nullptr, Ui::scanner_gui *mui = nullptr);
    ~ScanWizard();

signals:
    void send_dut_name(QString);

private slots:

    //Checking slots for timers
    void check_for_height_measured(void);
    void check_for_pcb_selected(void);
    void check_for_height_meas_point_selected(void);
    void check_for_pcb_corner_selected(void);
    void check_for_pcb_ref_point_selected(void);
    void check_for_instruments_created();

    //Slots for the signals coming from scan interface
    void scan_finished(void);
    void scan_error(void);

    //Buttons slots
    void on_Next_button_clicked();
    void on_Cancel_button_clicked();
    void on_instruments_settings_clicked();
    void on_back_button_clicked();

    //DUT name slot to check for the minimum 2 characters input name
    void on_dut_name_textChanged(const QString &arg1);

    void closeEvent(QCloseEvent *event) override;

private:
    Ui::ScanWizard *ui;

    //Partent's widget and ui
    QWidget *par{};

    //Wizard step tracking
    int step{};
    bool back{false};

    //Timer for detecting
    std::unique_ptr<QTimer> timer;

    //Estimated time display functions/variables
    void est_time_init(void);
    void scan_procedure(void);
    void update_time(void);
    int s=0, m=0, h=0, time=0;
};

#endif // SCANWIZARD_H
