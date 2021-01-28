#ifndef MAINSETTINGS_H
#define MAINSETTINGS_H

#include <QDialog>
#include <QStringListModel>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include "robot.h"
#include "event_log.h"

namespace Ui {
class MainSettings;
}

class MainSettings : public QDialog
{
    Q_OBJECT

public:
    explicit MainSettings(QWidget *parent = nullptr, Robot *r = nullptr);
    ~MainSettings();
    void read_previous_values();

signals:
    void send_ref00_offsets(double, double);

private slots:
    void on_robot_ref00_x_offset_valueChanged(double arg1);
    void on_robot_ref00_y_offset_valueChanged(double arg1);
    void on_height_meas_x_offset_valueChanged(double arg1);
    void on_height_meas_y_offset_valueChanged(double arg1);
    void on_start_scan_x_offset_valueChanged(double arg1);
    void on_start_scan_y_offset_valueChanged(double arg1);
    void on_OK_button_clicked();
    void on_Cancel_button_clicked();
    void on_apply_button_clicked();
    void on_Settings_tab_widget_currentChanged(int index);

    void on_open_logfile_clicked();

private:
    Ui::MainSettings *ui;
    QSettings *mainsettings;
    Robot *robot;
    QStringListModel *model;
    void assign_offsets();
    void init_logfiles();
    double ref00_x, ref00_y, height_off_x, height_off_y, start_scan_off_x, start_scan_off_y;
};

#endif // MAINSETTINGS_H
