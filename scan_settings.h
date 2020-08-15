#ifndef SCAN_SETTINGS_H
#define SCAN_SETTINGS_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class scan_settings;
}

class scan_settings : public QDialog
{
    Q_OBJECT

public:
    explicit scan_settings(QTcpSocket *socket, QWidget *parent = nullptr);
    ~scan_settings();

private slots:

    void on_center_span_radiobutton_clicked();
    void on_start_stop_radiobutton_clicked();
    void on_apply_click();
    void on_referencelevel_checkbox_stateChanged(int arg1);
    void on_leveloffset_checkbox_stateChanged(int arg1);
    void on_attenuation_checkbox_stateChanged(int arg1);
    void on_sweep_checkbox_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);

    void on_buttonBox_accepted();

    void on_start_freq_spinbox_valueChanged(double arg1);

    void on_stop_freq_spinbox_valueChanged(double arg1);

    void on_center_freq_spinbox_valueChanged(double arg1);

    void on_start_freq_dropdown_currentIndexChanged(int index);

    void on_stop_freq_dropdown_currentIndexChanged(int index);

private:
    Ui::scan_settings *ui;
    QTcpSocket *_socket_sa;

    const double sa_max_freq = 3.2;
    const QString sa_max_freq_unit = "GHz";

};

#endif // SCAN_SETTINGS_H
