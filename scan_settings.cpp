#include "scan_settings.h"
#include "ui_scan_settings.h"
#include <QHostAddress>
#include "scanner_gui.h"
#include "QPushButton"
#include <string>
#include <QDate>
#include <QTime>

int center_freq;
char center_freq_str[80];
int start_freq;
char start_freq_str[80];
char read_buffer[128];

scan_settings::scan_settings(QTcpSocket *socket, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::scan_settings)
{
    ui->setupUi(this);
    _socket_sa = socket;
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this ,SLOT(on_apply_click()));

    //Default settings for the scan settings window
    ui->referencelevel_spinbox->setEnabled(false);
    ui->leveloffset_spinbox->setEnabled(false);
    ui->attenuation_spinbox->setEnabled(false);
    ui->sweepTime_spinbox->setEnabled(false);
    ui->center_freq_spinbox->setEnabled(false);
    ui->spanfreq_spinbox->setEnabled(false);
    ui->frequency_dropdown_center->setEnabled(false);
    ui->frequency_dropdown_span->setEnabled(false);
}

scan_settings::~scan_settings()
{
    delete ui;
}

void scan_settings::on_center_span_radiobutton_clicked() //Center frequency and span
{
    //Enabled
    ui->center_freq_spinbox->setEnabled(true);
    ui->spanfreq_spinbox->setEnabled(true);
    ui->frequency_dropdown_center->setEnabled(true);
    ui->frequency_dropdown_span->setEnabled(true);
    //Disabled
    ui->start_freq_spinbox->setEnabled(false);
    ui->start_freq_dropdown->setEnabled(false);
    ui->stop_freq_spinbox->setEnabled(false);
    ui->stop_freq_dropdown->setEnabled(false);

}

void scan_settings::on_start_stop_radiobutton_clicked()
{
    //enabled
    ui->start_freq_spinbox->setEnabled(true);
    ui->start_freq_dropdown->setEnabled(true);
    ui->stop_freq_spinbox->setEnabled(true);
    ui->stop_freq_dropdown->setEnabled(true);
    //disabled
    ui->center_freq_spinbox->setEnabled(false);
    ui->spanfreq_spinbox->setEnabled(false);
    ui->frequency_dropdown_center->setEnabled(false);
    ui->frequency_dropdown_span->setEnabled(false);
   // ui->start_freq_spinbox
}

void scan_settings::on_apply_click()
{
    QString mystring;

    // ************************************************************************************************************************ //
    // *** SSA3032X Menu -> Frequency *** //
    if(ui->center_span_radiobutton->isChecked())
    {
        // Center frequency
        mystring = ":FREQuency:CENTer %1 %2\n";
        mystring = mystring.arg(QString::number(ui->center_freq_spinbox->value()), ui->frequency_dropdown_center->currentText());
        send_command(mystring);
        mystring = "";

        // Span
        mystring = ":FREQuency:SPAN %1 %2\n";
        mystring = mystring.arg(QString::number(ui->spanfreq_spinbox->value()), ui->frequency_dropdown_span->currentText());
        send_command(mystring);
        mystring = "";
    }
    else if(ui->start_stop_radiobutton->isChecked())
    {
        // Start frequency
        mystring = ":FREQuency:STARt %1 %2\n";
        mystring = mystring.arg(QString::number(ui->start_freq_spinbox->value()), ui->start_freq_dropdown->currentText());
        send_command(mystring);
        mystring = "";

       // Stop frequency
        mystring = ":FREQuency:STOP %1 %2\n";
        mystring = mystring.arg(QString::number(ui->stop_freq_spinbox->value()), ui->stop_freq_dropdown->currentText());
        send_command(mystring);
        mystring = "";
    }
    // Step
    mystring = ":FREQuency:CENTer:STEP %1";
    mystring = mystring.arg(QString::number(ui->step_spinbox->value()));
    mystring = mystring + " MHz\n";
    send_command(mystring);
    mystring = "";
    // ************************************************************************************************************************ //

    // ************************************************************************************************************************ //
    // *** SSA3032X Menu -> Aplitude *** //
    if(ui->referencelevel_checkbox->isChecked())
    {
        // Reference level
        mystring = ":DISPlay:WINDow:TRACe:Y:RLEVel %1 %2\n";
        mystring = mystring.arg(QString::number(ui->step_spinbox->value()), ui->referencelevel_spinbox->suffix());
        send_command(mystring);
        mystring = "";
    }
    if(ui->attenuation_checkbox->isChecked())
    {
        // Attenuation
        mystring = ":POWer:ATTenuation %1\n";
        mystring = mystring.arg(QString::number(ui->attenuation_spinbox->value()));
        send_command(mystring);
        mystring = "";
    }
    if(ui->leveloffset_checkbox->isChecked())
    {
        // Level offset
        mystring = ":DISPlay:WINDow:TRACe:Y:SCALe:RLEVel:OFFSet %1\n";
        mystring = mystring.arg(QString::number(ui->leveloffset_spinbox->value()));
        send_command(mystring);
        mystring = "";
    }

    // Units
    mystring = ":UNIT:POWer %1\n";
    mystring = mystring.arg(ui->units_combobox->currentText().toUpper());
    send_command(mystring);
    mystring = "";

    // Scale
    if(ui->scaleCheckbox->isChecked())
         mystring = ":DISPlay:WINDow:TRACe:Y:SCALe:SPACing LOGarithmic\n";
    else
        mystring = ":DISPlay:WINDow:TRACe:Y:SCALe:SPACing LINear\n";
    send_command(mystring);
    mystring = "";
    // ************************************************************************************************************************ //

    // ************************************************************************************************************************ //
    // *** SSA3032X Menu -> Sweep *** //
    // Number of sweeps
    mystring = ":SWEep:COUNt %1\n";
    mystring = mystring.arg(QString::number(ui->no_sweeps_spinbox->value()));
    send_command(mystring);
    mystring = "";

    if(ui->sweepTime_checkbox->isChecked())
    {
        // Sweep time
        mystring = ":SWEep:TIME %1 ms\n";
        mystring = mystring.arg(QString::number(ui->sweepTime_spinbox->value()));
        send_command(mystring);
        mystring = "";
    }
    // ************************************************************************************************************************ //

    // ************************************************************************************************************************ //
    // *** SSA3032X Menu -> BW (Bandwidth) *** //
    // Resolution bandwidth
    mystring = ":BWIDth %1\n";
    mystring = mystring.arg(ui->resolutionBW_dropdown->currentText());
    send_command(mystring);
    mystring = "";

    if(ui->same_RBW_VBW_checkBox->isChecked())
    {
        mystring = ":BWIDth:VIDeo %1\n";
        mystring = mystring.arg(ui->resolutionBW_dropdown->currentText());
        send_command(mystring);
        mystring = "";
    }
    else
    {
        mystring = ":BWIDth:VIDeo %1\n";
        mystring = mystring.arg(ui->videoBW_dropdown->currentText());
        send_command(mystring);
        mystring = "";
    }
    // ************************************************************************************************************************ //
}

void scan_settings::send_command(const QString &cmd)
{
    qDebug() << cmd;
    _socket_sa->write(cmd.toLocal8Bit());
    _socket_sa->waitForBytesWritten(1);
}


void scan_settings::on_start_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= ui->stop_freq_spinbox->value() && ui->start_freq_dropdown->currentText() == ui->stop_freq_dropdown->currentText())
        ui->stop_freq_spinbox->setValue(arg1 + 1);

    if(arg1 >= sa_max_freq && ui->start_freq_dropdown->currentText() == sa_max_freq_unit)
        ui->start_freq_spinbox->setValue(sa_max_freq-0.1);
}

void scan_settings::on_stop_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= sa_max_freq && ui->stop_freq_dropdown->currentText() == sa_max_freq_unit)
        ui->stop_freq_spinbox->setValue(sa_max_freq);
}

void scan_settings::on_center_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= sa_max_freq && ui->frequency_dropdown_center->currentText() == sa_max_freq_unit)
        ui->center_freq_spinbox->setValue(sa_max_freq-0.1);

}

void scan_settings::on_start_freq_dropdown_currentIndexChanged(int index)
{
    if(ui->stop_freq_dropdown->currentIndex() > index)
        ui->stop_freq_dropdown->setCurrentIndex(index);
}

void scan_settings::on_stop_freq_dropdown_currentIndexChanged(int index)
{
    if(index > ui->start_freq_dropdown->currentIndex())
        ui->start_freq_dropdown->setCurrentIndex(ui->stop_freq_dropdown->currentIndex());
}

void scan_settings::on_units_combobox_currentIndexChanged(const QString &arg1)
{
    ui->referencelevel_spinbox->setSuffix(" " + arg1);
}

void scan_settings::on_buttonBox_accepted()
{
    emit on_apply_click();
}

void scan_settings::on_referencelevel_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->referencelevel_spinbox->setEnabled(false);
    else
        ui->referencelevel_spinbox->setEnabled(true);
}

void scan_settings::on_leveloffset_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->leveloffset_spinbox->setEnabled(false);
    else
        ui->leveloffset_spinbox->setEnabled(true);
}

void scan_settings::on_attenuation_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->attenuation_spinbox->setEnabled(false);
    else
        ui->attenuation_spinbox->setEnabled(true);
}

void scan_settings::on_sweepTime_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->sweepTime_spinbox->setEnabled(false);
    else
        ui->sweepTime_spinbox->setEnabled(true);
}

void scan_settings::on_same_RBW_VBW_checkBox_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->videoBW_dropdown->setEnabled(true);
    }
    else
    {
        ui->videoBW_dropdown->setEnabled(false);
    }
}

