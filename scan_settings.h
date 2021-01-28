#ifndef SCAN_SETTINGS_H
#define SCAN_SETTINGS_H

#include <QDialog>
#include <QTcpSocket>
#include <QSettings>
#include <QCoreApplication>
#include "rs_instruments.h"

namespace Ui {
class scan_settings;
}

class scan_settings : public QDialog
{
    Q_OBJECT

public:
    explicit scan_settings(RS_Instruments *ins, bool sa_vna, QWidget *parent = nullptr);
    ~scan_settings();

signals:
    void send_sweep_points_amount(int);

private slots:

    void on_center_span_radiobutton_clicked();
    void on_start_stop_radiobutton_clicked();
    void on_apply_clicked();
    void on_referencelevel_checkbox_stateChanged(int arg1);
    void on_leveloffset_checkbox_stateChanged(int arg1);
    void on_attenuation_checkbox_stateChanged(int arg1);
    void on_sweepTime_checkbox_stateChanged(int arg1);
    void on_same_RBW_VBW_checkBox_stateChanged(int arg1);
    void on_start_freq_spinbox_valueChanged(double arg1);
    void on_stop_freq_spinbox_valueChanged(double arg1);
    void on_center_freq_spinbox_valueChanged(double arg1);
    void on_start_freq_dropdown_currentIndexChanged(int index);
    void on_stop_freq_dropdown_currentIndexChanged(int index);
    void on_units_combobox_currentIndexChanged(const QString &arg1);
    void on_use_instrument_settings_stateChanged(int arg1);
    void on_frequency_dropdown_center_currentIndexChanged(int index);
    void on_spanfreq_spinbox_valueChanged(double arg1);
    void on_frequency_dropdown_span_currentIndexChanged(int index);
    void on_attenuation_spinbox_valueChanged(int arg1);
    void on_leveloffset_spinbox_valueChanged(int arg1);
    void on_scaleCheckbox_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_clicked();
    void on_referencelevel_spinbox_valueChanged(double arg1);
    void on_resolutionBW_comboBox_currentIndexChanged(int index);
    void on_no_sweeps_spinbox_valueChanged(int arg1);
    void on_sweep_points_spinbox_valueChanged(int arg1);
    void on_sweepTime_spinbox_valueChanged(double arg1);
    void on_use_signalGen_checkbox_stateChanged(int arg1);
    void on_center_span_radiobutton_VNA_toggled(bool checked);
    void on_referencelevel_checkbox_VNA_stateChanged(int arg1);
    void on_attenuation_checkbox_VNA_stateChanged(int arg1);
    void on_leveloffset_checkbox_VNA_stateChanged(int arg1);
    void on_sweepTime_checkbox_VNA_stateChanged(int arg1);
    void on_start_freq_value_VNA_valueChanged(double arg1);
    void on_start_freq_unit_VNA_currentIndexChanged(int index);
    void on_stop_freq_value_VNA_valueChanged(double arg1);
    void on_stop_freq_unit_VNA_currentIndexChanged(int index);
    void on_center_freq_value_VNA_valueChanged(double arg1);
    void on_center_freq_unit_VNA_currentIndexChanged(int index);
    void on_span_freq_value_VNA_valueChanged(double arg1);
    void on_span_freq_unit_VNA_currentIndexChanged(int index);
    void on_referencelevel_spinbox_VNA_valueChanged(int arg1);
    void on_attenuation_spinbox_VNA_valueChanged(int arg1);
    void on_leveloffset_spinbox_VNA_valueChanged(int arg1);
    void on_scaleCheckbox_VNA_stateChanged(int arg1);
    void on_sweep_points_spinbox_VNA_valueChanged(int arg1);
    void on_sweepTime_spinbox_VNA_valueChanged(double arg1);
    void on_resolutionBW_comboBox_VNA_currentIndexChanged(int index);
    void on_no_sweeps_dropdown_currentIndexChanged(int index);
    void on_detectorComboBox_currentIndexChanged(int index);
    void on_preamp_on_checkbox_stateChanged(int arg1);

private:
    Ui::scan_settings *ui;

    RS_Instruments *instrument;

    QTcpSocket *_socket_sa;
    QTcpSocket *_socket_vna;

    QString settingsFile;
    QSettings *scansettings;

    const QString settings_file_path =  QCoreApplication::applicationDirPath() + "/settings/scansettings.ini";
    const double sa_max_freq = 7.5;
    const uint8_t sa_max_freq_unit = 0; // Dropdown index

    void sa_send_command(const QString &cmd);
    void vna_send_command(const QString &cmd);

    bool instrument_settings = false;
    void load_sa_previous_settings();
    void load_vna_previous_settings();
    void write_sa_settings();
    void write_vna_settings();
};

#endif // SCAN_SETTINGS_H
