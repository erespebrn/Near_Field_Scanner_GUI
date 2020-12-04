#include "scan_settings.h"
#include "ui_scan_settings.h"
#include <QHostAddress>
#include "scanner_gui.h"
#include "QPushButton"
#include <string>
#include <QDate>
#include <QTime>
#include <QFile>
#include <QDebug>


scan_settings::scan_settings(QTcpSocket *socket, bool sa_vna, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::scan_settings)
{
    ui->setupUi(this);

    if(sa_vna == false)
    {
        _socket_sa = socket;
        ui->VNA_tab->setEnabled(false);
    }
    else
    {
        _socket_vna = socket;
        ui->SA_tab->setEnabled(false);
    }

    load_previous_settings();

}

scan_settings::scan_settings(QTcpSocket *socket, QTcpSocket *socket2, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::scan_settings)
{
    ui->setupUi(this);

    _socket_sa = socket;
    _socket_vna = socket2;

    load_previous_settings();
}


scan_settings::~scan_settings()
{
    delete ui;
}


void scan_settings::on_apply_clicked()
{
    if(ui->tabWidget->currentIndex() == 0)
        write_sa_settings();
    else if(ui->tabWidget->currentIndex() == 1)
        write_vna_settings();

    ui->apply->setEnabled(false);
}

void scan_settings::write_sa_settings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/scansettings.ini", QSettings::IniFormat);
    settings.clear();
    QString mystring;

    if(!instrument_settings)
    {
        // *** FPL1007 Menu -> Frequency *** //
        if(ui->start_stop_radiobutton->isChecked())
        {
            // Start frequency //
            // Send SPCI command
            mystring = "FREQ:STAR %1 %2\n";
            mystring = mystring.arg(QString::number(ui->start_freq_spinbox->value()), ui->start_freq_dropdown->currentText());
            sa_send_command(mystring);
            mystring = "";

            // Stop frequency //
            // Send SPCI command
            mystring = "FREQ:STOP %1 %2\n";
            mystring = mystring.arg(QString::number(ui->stop_freq_spinbox->value()), ui->stop_freq_dropdown->currentText());
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_FREQUENCY/StartStop?", ui->start_stop_radiobutton->isChecked());
            settings.setValue("SA_FREQUENCY/Start freq. value", ui->start_freq_spinbox->value());
            settings.setValue("SA_FREQUENCY/Start freq. unit", ui->start_freq_dropdown->currentIndex());
            settings.setValue("SA_FREQUENCY/Stop freq. value", ui->stop_freq_spinbox->value());
            settings.setValue("SA_FREQUENCY/Stop freq. unit", ui->stop_freq_dropdown->currentIndex());
        }
        else if(ui->center_span_radiobutton->isChecked())
        {
            // Center frequency
            // Send SPCI command
            mystring = "FREQ:CENT %1 %2\n";
            mystring = mystring.arg(QString::number(ui->center_freq_spinbox->value()), ui->frequency_dropdown_center->currentText());
            sa_send_command(mystring);
            mystring = "";

            // Span
            // Send SPCI command
            mystring = "FREQ:SPAN %1 %2\n";
            mystring = mystring.arg(QString::number(ui->spanfreq_spinbox->value()), ui->frequency_dropdown_span->currentText());
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_FREQUENCY/CenterSpan?", ui->center_span_radiobutton->isChecked());
            settings.setValue("SA_FREQUENCY/Center freq. value", ui->center_freq_spinbox->value());
            settings.setValue("SA_FREQUENCY/Center freq. unit", ui->frequency_dropdown_center->currentIndex());
            settings.setValue("SA_FREQUENCY/Span freq. value", ui->spanfreq_spinbox->value());
            settings.setValue("SA_FREQUENCY/Span freq. unit", ui->frequency_dropdown_span->currentIndex());
        }

        // Save to the preset file
        settings.setValue("SA_FREQUENCY/Step freq. unit", "MHz");
        // ************************************************************************************************************************ //

        // ************************************************************************************************************************ //
        // *** FPL1007 Menu -> Amplitude *** //
        if(ui->referencelevel_checkbox->isChecked())
        {
            // Reference level
            // Send SPCI command
            mystring = "DISP:TRAC:Y:RLEV %1%2\n";
            mystring = mystring.arg(QString::number(ui->referencelevel_spinbox->value()), ui->referencelevel_spinbox->suffix());
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_AMPLITUDE/Ref. Level?", ui->referencelevel_checkbox->isChecked());
            settings.setValue("SA_AMPLITUDE/Ref. level value",  ui->referencelevel_spinbox->value());
        }

        if(ui->attenuation_checkbox->isChecked())
        {
            // Attenuation
            // Send SPCI command
            mystring = "INP:ATT %1\n";
            mystring = mystring.arg(QString::number(ui->attenuation_spinbox->value()));
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_AMPLITUDE/Attenuation?", ui->attenuation_checkbox->isChecked());
            settings.setValue("SA_AMPLITUDE/Attenuation value",  ui->attenuation_spinbox->value());

        }
        else
        {
            _socket_sa->write("INP:ATT:AUTO ON\n");
            _socket_sa->waitForBytesWritten(20);
        }


        if(ui->leveloffset_checkbox->isChecked())
        {
            // Level offset
            // Send SPCI command
            mystring = "DISP:TRAC:Y:RLEV:OFFS %1\n";
            mystring = mystring.arg(QString::number(ui->leveloffset_spinbox->value()));
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_AMPLITUDE/Level offset?", ui->leveloffset_checkbox->isChecked());
            settings.setValue("SA_AMPLITUDE/Level offset value",  ui->leveloffset_spinbox->value());
        }

        // Units
        // Send SPCI command
        mystring = "UNIT:POW %1\n";
        mystring = mystring.arg(ui->units_combobox->currentText().toUpper());
        sa_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.setValue("SA_UNITS/Units",  ui->units_combobox->currentIndex());

        // Preamp
        if(ui->preamp_on_checkbox->isChecked())
        {
            // Send SPCI command
            mystring = "INP:GAIN:STAT ON\n";
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_PREAMP/Amp",true);
        }
        else
        {
            // Send SPCI command
            mystring = "INP:GAIN:STAT OFF\n";
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_PREAMP/Amp",false);
        }

        // Scale
        if(ui->scaleCheckbox->isChecked())
        {
            // Send SPCI command
            mystring = "DISP:TRAC:Y:SPAC LOG\n";
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_UNITS/LogScale",true);
        }
        else
        {
            // Send SPCI command
            mystring = "DISP:TRAC:Y:SPAC LIN\n";
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_UNITS/LogScale",false);
        }

        // ************************************************************************************************************************ //


        // ************************************************************************************************************************ //
        // *** FPL1007 Menu -> Sweep *** //
        // Save to the preset file

        // Number of sweeps
        // Send SPCI command
        mystring = "SWE:COUN %1\n";
        mystring = mystring.arg(QString::number(ui->no_sweeps_spinbox->value()));
        sa_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.setValue("SA_SWEEP/Number of sweeps",  ui->no_sweeps_spinbox->value());

        // Sweep points
        // Send SPCI command
        mystring = "SWE:POIN %1\n";
        mystring = mystring.arg(QString::number(ui->sweep_points_spinbox->value()));
        sa_send_command(mystring);
        emit send_sweep_points_amount(ui->sweep_points_spinbox->value());
        mystring = "";
        // Save to the preset file
        settings.setValue("SA_SWEEP/Sweep points", ui->sweep_points_spinbox->value());

        //Trace hold seeting
        //Send SPCI command
        mystring = "DISP:TRAC1:MODE " + ui->no_sweeps_dropdown->currentText() + "\n";
        sa_send_command(mystring);
        mystring = "";
        mystring = "DISP:WIND:TRAC1:MODE:HCON ON\n";
        sa_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.setValue("SA_SWEEP/Trace hold", ui->no_sweeps_dropdown->currentText());

        //Detector
        mystring = "DET " + ui->detectorComboBox->currentText() + "\n";
        sa_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.setValue("SA_SWEEP/Detector", ui->detectorComboBox->currentIndex());

        if(ui->sweepTime_checkbox->isChecked())
        {
            // Sweep time
            // Send SPCI command
            mystring = "SWE:TIME %1 ms\n";
            mystring = mystring.arg(QString::number(ui->sweepTime_spinbox->value()));
            sa_send_command(mystring);
            mystring = "";
            // Save to the preset file
            settings.setValue("SA_SWEEP/Sweep time?", ui->sweepTime_checkbox->isChecked());
            settings.setValue("SA_SWEEP/Sweep time",  ui->sweepTime_spinbox->value());
        }
        else
        {
            settings.setValue("SA_SWEEP/Sweep time?", ui->sweepTime_checkbox->isChecked());
        }

        // ************************************************************************************************************************ //

        // ************************************************************************************************************************ //
        // *** FPL1007 Menu -> BW (Bandwidth) *** //
        // Resolution bandwidth
        // Send SPCI command
        mystring = "BAND %1\n";
        mystring = mystring.arg(ui->resolutionBW_comboBox->currentText());
        sa_send_command(mystring);
        mystring = "";

        // Save to the preset file
        settings.setValue("SA_BANDWIDTH/ResolutionBW", ui->resolutionBW_comboBox->currentIndex());

        if(ui->same_RBW_VBW_checkBox->isChecked())
        {
            // Send SPCI command
            mystring = "BAND:VID %1\n";
            mystring = mystring.arg(ui->resolutionBW_comboBox->currentText());
            sa_send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("SA_BANDWIDTH/SameRBW_VBW?", ui->same_RBW_VBW_checkBox->isChecked());
            settings.setValue("SA_BANDWIDTH/VideoBW", ui->resolutionBW_comboBox->currentIndex());
        }
        else
        {
            // Send SPCI command
            mystring = "BAND:VID %1\n";
            mystring = mystring.arg(ui->videoBW_comboBox->currentText());
            sa_send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("SA_BANDWIDTH/SameRBW_VBW?", ui->same_RBW_VBW_checkBox->isChecked());
            settings.setValue("SA_BANDWIDTH/VideoBW", ui->videoBW_comboBox->currentIndex());
        }
        // ************************************************************************************************************************ //*/

        // *** Detector ***
        mystring = "DET %1\n";
        mystring = mystring.arg(ui->detectorComboBox->currentText());
        sa_send_command(mystring);
        mystring = "";
    }
}

void scan_settings::write_vna_settings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/scansettings.ini", QSettings::IniFormat);

    QString mystring = "";

    // Delete all existing traces in VNA
    mystring = "CALC:PAR:DEl:ALL\n";
    vna_send_command(mystring);
    mystring = "";

    // Create trace called 'MeasTrace1' with S-Parameter S12
    mystring = "CALC1:PAR:SDEF 'MeasTrace1', 'S12'\n";
    vna_send_command(mystring);
    mystring = "";

    // Display this trace on Window 1
    mystring = "DISP:WIND1:TRAC:EFE 'MeasTrace1'\n";
    vna_send_command(mystring);
    mystring = "";

    // *** VNA ZNB40 *** //

    // **************************************************** VNA -> FREQUENCY ******************************************************** //
    if(ui->start_stop_radiobutton_VNA->isChecked())
    {
        // Start frequency
        // SPCI command
        mystring = "SENS1:FREQ:STAR %1 %2\n";
        mystring = mystring.arg(QString::number(ui->start_freq_value_VNA->value()), ui->start_freq_unit_VNA->currentText());
        vna_send_command(mystring);
        mystring = "";
        // Stop frequency
        // SPCI command
        mystring = "SENS1:FREQ:STOP %1 %2\n";
        mystring = mystring.arg(QString::number(ui->stop_freq_value_VNA->value()), ui->stop_freq_unit_VNA->currentText());
        vna_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.value("VNA_FREQUENCY/StartStop?", ui->start_stop_radiobutton_VNA->isChecked());
        settings.value("VNA_FREQUENCY/CenterSpan?", ui->center_span_radiobutton_VNA->isChecked());
        settings.value("VNA_FREQUENCY/Start freq. value", ui->start_freq_value_VNA->value());
        settings.value("VNA_FREQUENCY/Start freq. unit", ui->start_freq_unit_VNA->currentText());
        settings.value("VNA_FREQUENCY/Stop freq. value", ui->stop_freq_value_VNA->value());
        settings.value("VNA_FREQUENCY/Stop freq. unit", ui->stop_freq_unit_VNA->currentText());
    }
    else if(ui->center_span_radiobutton_VNA->isChecked())
    {
        // Center frequency
        // SPCI command
        mystring = "SENS1:FREQ:CENT %1 %2\n";
        mystring = mystring.arg(QString::number(ui->center_freq_value_VNA->value()), ui->center_freq_unit_VNA->currentText());
        vna_send_command(mystring);
        mystring = "";
        // Span
        // SPCI command
        mystring = "SENS1:FREQ:SPAN %1 %2\n";
        mystring = mystring.arg(QString::number(ui->span_freq_value_VNA->value()), ui->span_freq_unit_VNA->currentText());
        vna_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.value("VNA_FREQUENCY/StartStop?", ui->start_stop_radiobutton_VNA->isChecked());
        settings.value("VNA_FREQUENCY/CenterSpan?", ui->center_span_radiobutton_VNA->isChecked());
        settings.value("VNA_FREQUENCY/Center freq. value", ui->center_freq_value_VNA->value());
        settings.value("VNA_FREQUENCY/Center freq. unit", ui->center_freq_unit_VNA->currentText());
        settings.value("VNA_FREQUENCY/Span freq. value", ui->span_freq_value_VNA->value());
        settings.value("VNA_FREQUENCY/Span freq. unit", ui->span_freq_unit_VNA->currentText());
    }

    // ********************************************************************************************************************************* //

    // ****************************************************** VNA -> SCALE ************************************************************ //
    if(ui->referencelevel_checkbox_VNA->isChecked())
    {
        // Reference level
        mystring = "DISP:WIND1:TRAC:Y:RLEV %1\n";
        mystring = mystring.arg(QString::number(ui->referencelevel_spinbox_VNA->value()));
        vna_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.value("VNA_SCALE/Reference level", ui->referencelevel_spinbox_VNA->value());
    }

    // ********************************************************************************************************************************* //

    // ****************************************************** VNA -> SWEEP ************************************************************ //
    // Sweep points
    mystring = "SWE:POIN %1\n";
    mystring = mystring.arg(ui->sweep_points_spinbox_VNA->value());
    vna_send_command(mystring);
    mystring = "";
    // Save to the preset file
    settings.value("VNA_SWEEP/Sweep points", ui->sweep_points_spinbox_VNA->value());

    if(ui->sweepTime_checkbox_VNA->isChecked())
    {
        // Sweep time
        mystring = "SWE:TIME %1\n";
        mystring = mystring.arg(ui->sweepTime_spinbox_VNA->value());
        vna_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.value("VNA_SWEEP/Sweep points", ui->sweepTime_spinbox_VNA->value());
    }

    if(ui->scaleCheckbox_VNA->isChecked())
    {
        // Log freq.
        mystring = "SWE:TYPE LOG\n";
        vna_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.value("VNA_SWEEP/LogFreq?", ui->scaleCheckbox_VNA->isChecked());
    }
    else
    {
        // Log freq.
        mystring = "SWE:TYPE LIN\n";
        vna_send_command(mystring);
        mystring = "";
        // Save to the preset file
        settings.value("VNA_SWEEP/LogFreq?", ui->scaleCheckbox_VNA->isChecked());
    }
    // ********************************************************************************************************************************* //

    // ****************************************************** VNA -> IF ************************************************************ //
    // Sweep points
    mystring = "BAND %1\n";
    mystring = mystring.arg(ui->resolutionBW_comboBox_VNA->currentText());
    vna_send_command(mystring);
    mystring = "";
    // Save to the preset file
    settings.value("VNA_SWEEP/Sweep points", ui->resolutionBW_comboBox_VNA->currentText());
    // ********************************************************************************************************************************* //
}

void scan_settings::load_previous_settings()
{
    QFile settings_file(settings_file_path);
    if(settings_file.exists())
    {
        QSettings settings(settings_file_path, QSettings::IniFormat);

        // Load last settings from .ini file
        // *** FREQUENCY *** //
        if(settings.value("SA_FREQUENCY/StartStop?").toBool())
        {
            ui->start_stop_radiobutton->setChecked(true);
            ui->center_span_radiobutton->setChecked(false);
            ui->center_freq_spinbox->setEnabled(false);
            ui->spanfreq_spinbox->setEnabled(false);
            ui->frequency_dropdown_center->setEnabled(false);
            ui->frequency_dropdown_span->setEnabled(false);

            // Start frequency
            ui->start_freq_dropdown->setCurrentIndex(settings.value("SA_FREQUENCY/Start freq. unit").toInt());
            ui->start_freq_spinbox->setValue(settings.value("SA_FREQUENCY/Start freq. value").toDouble());

            // Stop frequency
            ui->stop_freq_dropdown->setCurrentIndex(settings.value("SA_FREQUENCY/Stop freq. unit").toInt());
            ui->stop_freq_spinbox->setValue(settings.value("SA_FREQUENCY/Stop freq. value").toDouble());
        }
        if(settings.value("SA_FREQUENCY/CenterSpan?").toBool())
        {
            ui->center_span_radiobutton->setChecked(true);
            ui->start_stop_radiobutton->setChecked(false);
            ui->start_freq_spinbox->setEnabled(false);
            ui->stop_freq_spinbox->setEnabled(false);
            ui->start_freq_dropdown->setEnabled(false);
            ui->stop_freq_dropdown->setEnabled(false);

            // Center frequency
            ui->frequency_dropdown_center->setCurrentIndex(settings.value("SA_FREQUENCY/Center freq. unit").toInt());
            ui->center_freq_spinbox->setValue(settings.value("SA_FREQUENCY/Center freq. value").toDouble());

            // Span frequency
            ui->frequency_dropdown_span->setCurrentIndex(settings.value("SA_FREQUENCY/Span freq. unit").toInt());
            ui->spanfreq_spinbox->setValue(settings.value("SA_FREQUENCY/Span freq. value").toDouble());
        }


        // *** Amplitude *** //
        if(settings.value("SA_AMPLITUDE/Ref. level?").toBool())
        {
            // Reference level
            ui->referencelevel_checkbox->setChecked(true);
            ui->referencelevel_spinbox->setValue(settings.value("SA_AMPLITUDE/Ref. level value").toDouble());
        }
        else
        {
            ui->referencelevel_spinbox->setEnabled(false);
        }
        if(settings.value("SA_AMPLITUDE/Attenuation?").toBool())
        {
            // Attenuation
            ui->attenuation_checkbox->setChecked(true);
            ui->attenuation_spinbox->setValue(settings.value("SA_AMPLITUDE/Attenuation value").toInt());
        }
        else
        {
            ui->attenuation_spinbox->setEnabled(false);
        }
        if(settings.value("SA_AMPLITUDE/Level offset?").toBool())
        {
            // Level offset
            ui->leveloffset_checkbox->setChecked(true);
            ui->leveloffset_spinbox->setValue(settings.value("SA_AMPLITUDE/Level offset value").toInt());
        }
        else
        {
            ui->leveloffset_spinbox->setEnabled(false);
        }
        if(settings.value("SA_PREAMP/Amp?").toBool())
        {
            // Preamp
            ui->preamp_on_checkbox->setChecked(true);
        }
        else
        {
            ui->preamp_on_checkbox->setChecked(false);
        }

        //Detector
        ui->detectorComboBox->setCurrentIndex(settings.value("SA_SWEEP/Detector").toInt());

        // Units
        ui->units_combobox->setCurrentIndex(settings.value("SA_UNITS/Units").toInt());
        // Scale
        ui->scaleCheckbox->setChecked(settings.value("SA_UNITS/LogScale").toBool());
        // *** End of AMPLITUDE *** //

        // *** Sweep *** //
        // Numbers of sweeps
        ui->no_sweeps_spinbox->setValue(settings.value("SA_SWEEP/Number of sweeps").toInt());

        // Sweep points
        ui->sweep_points_spinbox->setValue(settings.value("SA_SWEEP/Sweep points").toInt());

        if(settings.value("SA_SWEEP/Sweep time?").toBool())
        {
            // Sweep time
            ui->sweepTime_checkbox->setChecked(true);
            ui->sweepTime_spinbox->setValue(settings.value("SA_SWEEP/Sweep time").toDouble());
        }
        else
        {
            ui->sweepTime_checkbox->setChecked(false);
        }
        // *** End of sweep *** //

        // *** Video Bandwidth *** //
        ui->resolutionBW_comboBox->setCurrentIndex(settings.value("SA_BANDWIDTH/ResolutionBW").toInt());

        if(settings.value("SA_BANDWIDTH/SameRBW_VBW").toBool())
        {
            ui->same_RBW_VBW_checkBox->setChecked(true);
        }
        else
        {
            ui->same_RBW_VBW_checkBox->setChecked(false);
        }

        ui->videoBW_comboBox->setCurrentIndex(settings.value("SA_BANDWIDTH/VideoBW").toInt());
    }
    else
    {
        //Default settings for the scan settings window
        ui->videoBW_comboBox->setEnabled(false);
        ui->referencelevel_spinbox->setEnabled(false);
        ui->leveloffset_spinbox->setEnabled(false);
        ui->attenuation_spinbox->setEnabled(false);
        ui->sweepTime_spinbox->setEnabled(false);
        ui->center_freq_spinbox->setEnabled(false);
        ui->spanfreq_spinbox->setEnabled(false);
        ui->preamp_on_checkbox->setChecked(false);
        ui->frequency_dropdown_center->setEnabled(false);
        ui->frequency_dropdown_span->setEnabled(false);
    }
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

    ui->apply->setEnabled(true);
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

    ui->apply->setEnabled(true);
}

void scan_settings::sa_send_command(const QString &cmd)
{
    _socket_sa->write(cmd.toLocal8Bit());
    _socket_sa->waitForBytesWritten(1);
}

void scan_settings::vna_send_command(const QString &cmd)
{
    _socket_vna->write(cmd.toLocal8Bit());
    _socket_vna->waitForBytesWritten(1);
}


void scan_settings::on_start_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= ui->stop_freq_spinbox->value() && ui->start_freq_dropdown->currentText() == ui->stop_freq_dropdown->currentText())
        ui->stop_freq_spinbox->setValue(arg1 + 1);

    if(arg1 >= sa_max_freq && ui->start_freq_dropdown->currentIndex() == sa_max_freq_unit)
        ui->start_freq_spinbox->setValue(sa_max_freq-0.1);

    ui->apply->setEnabled(true);
}

void scan_settings::on_stop_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= sa_max_freq && ui->stop_freq_dropdown->currentIndex() == sa_max_freq_unit)
        ui->stop_freq_spinbox->setValue(sa_max_freq);

    ui->apply->setEnabled(true);
}

void scan_settings::on_center_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= sa_max_freq && ui->frequency_dropdown_center->currentIndex() == sa_max_freq_unit)
        ui->center_freq_spinbox->setValue(sa_max_freq-0.1);

    ui->apply->setEnabled(true);
}

void scan_settings::on_start_freq_dropdown_currentIndexChanged(int index)
{
    if(ui->start_freq_spinbox->value() > sa_max_freq && index == sa_max_freq_unit)
        ui->start_freq_spinbox->setValue(sa_max_freq-0.1);
    if(ui->stop_freq_dropdown->currentIndex() > index)
        ui->stop_freq_dropdown->setCurrentIndex(index);

    ui->apply->setEnabled(true);
}

void scan_settings::on_stop_freq_dropdown_currentIndexChanged(int index)
{
    if(ui->stop_freq_spinbox->value() > sa_max_freq && index == sa_max_freq_unit)
        ui->stop_freq_spinbox->setValue(sa_max_freq);
    if(index > ui->start_freq_dropdown->currentIndex())
        ui->start_freq_dropdown->setCurrentIndex(ui->stop_freq_dropdown->currentIndex());
}

void scan_settings::on_units_combobox_currentIndexChanged(const QString &arg1)
{
    ui->referencelevel_spinbox->setSuffix(" " + arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_referencelevel_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->referencelevel_spinbox->setEnabled(false);
    else
        ui->referencelevel_spinbox->setEnabled(true);

    ui->apply->setEnabled(true);
}

void scan_settings::on_leveloffset_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->leveloffset_spinbox->setEnabled(false);
    else
        ui->leveloffset_spinbox->setEnabled(true);

    ui->apply->setEnabled(true);
}

void scan_settings::on_attenuation_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->attenuation_spinbox->setEnabled(false);
    }
    else
        ui->attenuation_spinbox->setEnabled(true);

    ui->apply->setEnabled(true);
}

void scan_settings::on_sweepTime_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->sweepTime_spinbox->setEnabled(false);
    else
        ui->sweepTime_spinbox->setEnabled(true);

    ui->apply->setEnabled(true);
}

void scan_settings::on_same_RBW_VBW_checkBox_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->videoBW_comboBox->setEnabled(true);
    }
    else
    {
        ui->videoBW_comboBox->setEnabled(false);
    }
}


void scan_settings::on_use_instrument_settings_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        instrument_settings = false;
        ui->frequencyFrame->setEnabled(true);
        ui->amplitudeFrame->setEnabled(true);
        ui->videoBW_Frame->setEnabled(true);
        ui->sweep_gridFrame->setEnabled(true);
        ui->detectorComboBox->setEnabled(true);
    }
    else
    {
        instrument_settings = true;
        ui->frequencyFrame->setEnabled(false);
        ui->amplitudeFrame->setEnabled(false);
        ui->videoBW_Frame->setEnabled(false);
        ui->sweep_gridFrame->setEnabled(false);
        ui->detectorComboBox->setEnabled(false);
    }
}


void scan_settings::on_scaleCheckbox_clicked()
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_pushButton_3_clicked()
{
    if(ui->apply->isEnabled())
    {
        this->hide();
        emit on_apply_clicked();
    }
    else
        this->hide();
}

void scan_settings::on_pushButton_clicked()
{
    this->hide();
}


void scan_settings::on_use_signalGen_checkbox_stateChanged(int arg1)
{
    ui->signalGen_start_freq_value->setEnabled(arg1);
    ui->signalGen_start_freq_unit->setEnabled(arg1);
    ui->signalGen_stop_freq_value->setEnabled(arg1);
    ui->signalGen_stop_freq_unit->setEnabled(arg1);
    ui->signalGen_power_value->setEnabled(arg1);

    ui->apply->setEnabled(true);
}

void scan_settings::on_center_span_radiobutton_VNA_toggled(bool checked)
{
    ui->start_freq_value_VNA->setEnabled(!checked);
    ui->start_freq_unit_VNA->setEnabled(!checked);
    ui->stop_freq_unit_VNA->setEnabled(!checked);
    ui->stop_freq_value_VNA->setEnabled(!checked);

    ui->center_freq_unit_VNA->setEnabled(checked);
    ui->center_freq_value_VNA->setEnabled(checked);
    ui->span_freq_unit_VNA->setEnabled(checked);
    ui->span_freq_value_VNA->setEnabled(checked);

    ui->apply->setEnabled(true);
}


void scan_settings::on_frequency_dropdown_center_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_spanfreq_spinbox_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_frequency_dropdown_span_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}



void scan_settings::on_attenuation_spinbox_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_leveloffset_spinbox_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_referencelevel_spinbox_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_resolutionBW_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_no_sweeps_spinbox_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_sweep_points_spinbox_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_sweepTime_spinbox_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_referencelevel_checkbox_VNA_stateChanged(int arg1)
{
    ui->referencelevel_spinbox_VNA->setEnabled(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_attenuation_checkbox_VNA_stateChanged(int arg1)
{
    ui->attenuation_spinbox_VNA->setEnabled(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_leveloffset_checkbox_VNA_stateChanged(int arg1)
{
    ui->leveloffset_spinbox_VNA->setEnabled(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_sweepTime_checkbox_VNA_stateChanged(int arg1)
{
    ui->sweepTime_spinbox_VNA->setEnabled(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_start_freq_value_VNA_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_start_freq_unit_VNA_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_stop_freq_value_VNA_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_stop_freq_unit_VNA_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_center_freq_value_VNA_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_center_freq_unit_VNA_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_span_freq_value_VNA_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_span_freq_unit_VNA_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_referencelevel_spinbox_VNA_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_attenuation_spinbox_VNA_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_leveloffset_spinbox_VNA_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_scaleCheckbox_VNA_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_sweep_points_spinbox_VNA_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_sweepTime_spinbox_VNA_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}

void scan_settings::on_resolutionBW_comboBox_VNA_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_no_sweeps_dropdown_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_detectorComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->apply->setEnabled(true);
}

void scan_settings::on_preamp_on_checkbox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    ui->apply->setEnabled(true);
}
