#include "scan_settings.h"
#include "ui_scan_settings.h"
#include <QHostAddress>
#include "scanner_gui.h"
#include "QPushButton"
#include <string>
#include <QDate>
#include <QTime>
#include <QFile>


scan_settings::scan_settings(QTcpSocket *socket, bool sa_vna, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::scan_settings)
{
    ui->setupUi(this);

    if(sa_vna == false)
        _socket_sa = socket;
    else
        _socket_vna = socket;

    load_previous_settings();

}

scan_settings::scan_settings(QTcpSocket *socket, QTcpSocket *socket2, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::scan_settings)
{
    ui->setupUi(this);

    _socket_sa = socket;
    _socket_vna = socket2;
}


scan_settings::~scan_settings()
{
    delete ui;
}


void scan_settings::on_apply_clicked()
{
    qDebug() << "here";
    QSettings settings(QCoreApplication::applicationDirPath() + "/scansettings.ini", QSettings::IniFormat);
    settings.clear();
    QString mystring;

    if(!instrument_settings)
    {
        // *** SSA3032X Menu -> Frequency *** //
        if(ui->start_stop_radiobutton->isChecked())
        {
            // Start frequency //
            // Send SPCI command
            mystring = "FREQ:STAR %1 %2\n";
            mystring = mystring.arg(QString::number(ui->start_freq_spinbox->value()), ui->start_freq_dropdown->currentText());
            send_command(mystring);
            mystring = "";

            // Stop frequency //
            // Send SPCI command
            mystring = "FREQ:STOP %1 %2\n";
            mystring = mystring.arg(QString::number(ui->stop_freq_spinbox->value()), ui->stop_freq_dropdown->currentText());
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("FREQUENCY/StartStop?", ui->start_stop_radiobutton->isChecked());
            settings.setValue("FREQUENCY/Start freq. value", ui->start_freq_spinbox->value());
            settings.setValue("FREQUENCY/Start freq. unit", ui->start_freq_dropdown->currentIndex());
            settings.setValue("FREQUENCY/Stop freq. value", ui->stop_freq_spinbox->value());
            settings.setValue("FREQUENCY/Stop freq. unit", ui->stop_freq_dropdown->currentIndex());
        }
        else if(ui->center_span_radiobutton->isChecked())
        {
            // Center frequency
            // Send SPCI command
            mystring = "FREQ:CENT %1 %2\n";
            mystring = mystring.arg(QString::number(ui->center_freq_spinbox->value()), ui->frequency_dropdown_center->currentText());
            send_command(mystring);
            mystring = "";

            // Span
            // Send SPCI command
            mystring = "FREQ:SPAN %1 %2\n";
            mystring = mystring.arg(QString::number(ui->spanfreq_spinbox->value()), ui->frequency_dropdown_span->currentText());
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("FREQUENCY/CenterSpan?", ui->center_span_radiobutton->isChecked());
            settings.setValue("FREQUENCY/Center freq. value", ui->center_freq_spinbox->value());
            settings.setValue("FREQUENCY/Center freq. unit", ui->frequency_dropdown_center->currentIndex());
            settings.setValue("FREQUENCY/Span freq. value", ui->spanfreq_spinbox->value());
            settings.setValue("FREQUENCY/Span freq. unit", ui->frequency_dropdown_span->currentIndex());
        }
        // Step
        // Send SPCI command
        mystring = "FREQ:CENT:STEP %1";
        mystring = mystring.arg(QString::number(ui->step_spinbox->value()));
        mystring = mystring + " MHz\n";
        send_command(mystring);
        mystring = "";

        // Save to the preset file
        settings.setValue("FREQUENCY/Step freq. value", ui->step_spinbox->value());
        settings.setValue("FREQUENCY/Step freq. unit", "MHz");
        // ************************************************************************************************************************ //

        // ************************************************************************************************************************ //
        // *** SSA3032X Menu -> Amplitude *** //
        if(ui->referencelevel_checkbox->isChecked())
        {
            // Reference level
            // Send SPCI command
            mystring = "DISPl:TRAC:Y:RLEV %1 %2\n";
            mystring = mystring.arg(QString::number(ui->step_spinbox->value()), ui->referencelevel_spinbox->suffix());
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("AMPLITUDE/Ref. Level?", ui->referencelevel_checkbox->isChecked());
            settings.setValue("AMPLITUDE/Ref. level value",  ui->referencelevel_spinbox->value());
        }
        if(ui->attenuation_checkbox->isChecked())
        {
            // Attenuation
            // Send SPCI command
            mystring = "INP:ATT %1\n";
            mystring = mystring.arg(QString::number(ui->attenuation_spinbox->value()));
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("AMPLITUDE/Attenuation?", ui->attenuation_checkbox->isChecked());
            settings.setValue("AMPLITUDE/Attenuation value",  ui->attenuation_spinbox->value());

        }
        if(ui->leveloffset_checkbox->isChecked())
        {
            // Level offset
            // Send SPCI command
            mystring = "DISPl:TRAC:Y:RLEV:OFFS %1\n";
            mystring = mystring.arg(QString::number(ui->leveloffset_spinbox->value()));
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("AMPLITUDE/Level offset?", ui->leveloffset_checkbox->isChecked());
            settings.setValue("AMPLITUDE/Level offset value",  ui->leveloffset_spinbox->value());
        }

        // Units
        // Send SPCI command
        mystring = "UNIT:POW %1\n";
        mystring = mystring.arg(ui->units_combobox->currentText().toUpper());
        send_command(mystring);
        mystring = "";

        // Save to the preset file
        settings.setValue("UNITS/Units",  ui->units_combobox->currentIndex());

        // Scale
        if(ui->scaleCheckbox->isChecked())
        {
            // Send SPCI command
            mystring = "DISP:TRAC:Y:SPAC LOG\n";
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("UNITS/LogScale", true);
        }
        else
        {
            // Send SPCI command
            mystring = "DISP:TRAC:Y:SPAC LIN\n";
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("UNITS/LogScale", false);
        }

        // ************************************************************************************************************************ //


        // ************************************************************************************************************************ //
        // *** SSA3032X Menu -> Sweep *** //
        // Save to the preset file

        // Number of sweeps
        // Send SPCI command
        mystring = ":SWEep:COUNt %1\n";
        mystring = mystring.arg(QString::number(ui->no_sweeps_spinbox->value()));
        send_command(mystring);
        mystring = "";

        // Save to the preset file
        settings.setValue("SWEEP/Number of swppes",  ui->no_sweeps_spinbox->value());


        if(ui->sweepTime_checkbox->isChecked())
        {
            // Sweep time
            // Send SPCI command
            mystring = ":SWEep:TIME %1 ms\n";
            mystring = mystring.arg(QString::number(ui->sweepTime_spinbox->value()));
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("SWEEP/Sweep time?", ui->sweepTime_checkbox->isChecked());
            settings.setValue("SWEEP/Sweep time",  ui->no_sweeps_spinbox->value());
        }
        }
        // ************************************************************************************************************************ //

        // ************************************************************************************************************************ //
        // *** SSA3032X Menu -> BW (Bandwidth) *** //
        // Resolution bandwidth
        // Send SPCI command
        mystring = ":BWIDth %1\n";
        mystring = mystring.arg(ui->resolutionBW_dropdown->currentText());
        send_command(mystring);
        mystring = "";

        // Save to the preset file
        settings.setValue("BANDWIDTH/ResolutionBW", ui->resolutionBW_dropdown->currentIndex());

        if(ui->same_RBW_VBW_checkBox->isChecked())
        {
            // Send SPCI command
            mystring = ":BWIDth:VIDeo %1\n";
            mystring = mystring.arg(ui->resolutionBW_dropdown->currentText());
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("BANDWIDTH/SameRBW_VBW?", ui->same_RBW_VBW_checkBox->isChecked());
            settings.setValue("BANDWIDTH/VideoBW", ui->resolutionBW_dropdown->currentIndex());
        }
        else
        {
            // Send SPCI command
            mystring = ":BWIDth:VIDeo %1\n";
            mystring = mystring.arg(ui->videoBW_dropdown->currentText());
            send_command(mystring);
            mystring = "";

            // Save to the preset file
            settings.setValue("BANDWIDTH/SameRBW_VBW?", ui->same_RBW_VBW_checkBox->isChecked());
            settings.setValue("BANDWIDTH/VideoBW", ui->videoBW_dropdown->currentIndex());
        }
        // ************************************************************************************************************************ //*/

    ui->apply->setEnabled(false);
}

void scan_settings::load_previous_settings()
{

    QFile settings_file(settings_file_path);
    if(settings_file.exists())
    {
        QSettings settings(settings_file_path, QSettings::IniFormat);

        // Load last settings from .ini file
        // *** FREQUENCY *** //
        if(settings.value("FREQUENCY/StartStop?").toBool())
        {
            ui->start_stop_radiobutton->setChecked(true);
            ui->center_span_radiobutton->setChecked(false);
            ui->center_freq_spinbox->setEnabled(false);
            ui->spanfreq_spinbox->setEnabled(false);
            ui->frequency_dropdown_center->setEnabled(false);
            ui->frequency_dropdown_span->setEnabled(false);

            // Start frequency
            ui->start_freq_dropdown->setCurrentIndex(settings.value("FREQUENCY/Start freq. unit").toInt());
            ui->start_freq_spinbox->setValue(settings.value("FREQUENCY/Start freq. value").toDouble());

            // Stop frequency
            ui->stop_freq_dropdown->setCurrentIndex(settings.value("FREQUENCY/Stop freq. unit").toInt());
            ui->stop_freq_spinbox->setValue(settings.value("FREQUENCY/Stop freq. value").toDouble());
        }
        if(settings.value("FREQUENCY/CenterSpan?").toBool())
        {
            ui->center_span_radiobutton->setChecked(true);
            ui->start_stop_radiobutton->setChecked(false);
            ui->start_freq_spinbox->setEnabled(false);
            ui->stop_freq_spinbox->setEnabled(false);
            ui->start_freq_dropdown->setEnabled(false);
            ui->stop_freq_dropdown->setEnabled(false);

            // Center frequency
            ui->frequency_dropdown_center->setCurrentIndex(settings.value("FREQUENCY/Center freq. unit").toInt());
            ui->center_freq_spinbox->setValue(settings.value("FREQUENCY/Center freq. value").toDouble());

            // Span frequency
            ui->frequency_dropdown_span->setCurrentIndex(settings.value("FREQUENCY/Span freq. unit").toInt());
            ui->spanfreq_spinbox->setValue(settings.value("FREQUENCY/Span freq. value").toDouble());
        }

        // Step
        ui->step_spinbox->setValue(settings.value("FREQUENCY/Step freq. value").toInt());
        ui->step_spinbox->setSuffix(settings.value("FREQUENCY/Step freq. unit").toString());
        // *** End of FREQUENCY *** //

        // *** Amplitude *** //
        if(settings.value("AMPLITUDE/Ref. level?").toBool())
        {
            // Reference level
            ui->referencelevel_checkbox->setChecked(true);
            ui->referencelevel_spinbox->setValue(settings.value("AMPLITUDE/Ref. level value").toDouble());
        }
        else
        {
            ui->referencelevel_spinbox->setEnabled(false);
        }
        if(settings.value("AMPLITUDE/Attenuation?").toBool())
        {
            // Attenuation
            ui->attenuation_checkbox->setChecked(true);
            ui->attenuation_spinbox->setValue(settings.value("AMPLITUDE/Attenuation value").toInt());
        }
        else
        {
            ui->attenuation_spinbox->setEnabled(false);
        }
        if(settings.value("AMPLITUDE/Level offset?").toBool())
        {
            // Level offset
            ui->leveloffset_checkbox->setChecked(true);
            ui->leveloffset_spinbox->setValue(settings.value("AMPLITUDE/Level offset value").toInt());
        }
        else
        {
            ui->leveloffset_spinbox->setEnabled(false);
        }

        // Units
        ui->units_combobox->setCurrentIndex(settings.value("UNITS/Units").toInt());
        // Scale
        ui->scaleCheckbox->setChecked(settings.value("UNITS/LogScale").toBool());
        // *** End of AMPLITUDE *** //

        // *** Sweep *** //
        if(settings.value("SWEEP/SweepRadio?").toBool())
        {
           ui->no_sweeps_spinbox->setEnabled(true);
           ui->no_sweeps_dropdown->setEnabled(true);
           ui->sweep_points_spinbox->setEnabled(true);

           ui->sweepTime_checkbox->setChecked(false);
           ui->sweepTime_spinbox->setEnabled(false);
           ui->videoBW_dropdown->setEnabled(false);
           ui->same_RBW_VBW_checkBox->setEnabled(false);

           // Numbers of sweeps
           ui->no_sweeps_spinbox->setValue(settings.value("SWEEP/Number of sweeps").toInt());

           if(settings.value("SWEEP/Sweep time?").toBool())
           {
               // Sweep time
               ui->sweepTime_checkbox->setChecked(true);
               ui->sweepTime_spinbox->setValue(settings.value("SWEEP/Sweep time").toInt());
           }
        }
        // *** End of sweep *** //

        // *** Video Bandwidth *** //
        if(settings.value("BANDWIDTH/BandWidth?").toBool())
        {
            ui->no_sweeps_spinbox->setEnabled(false);
            ui->sweep_points_spinbox->setEnabled(false);
            ui->sweepTime_checkbox->setChecked(false);
            ui->sweepTime_checkbox->setEnabled(false);
            ui->sweepTime_spinbox->setEnabled(false);
            ui->no_sweeps_dropdown->setEnabled(false);

            ui->same_RBW_VBW_checkBox->setChecked(true);
            ui->videoBW_dropdown->setEnabled(true);
            ui->same_RBW_VBW_checkBox->setEnabled(true);

            ui->resolutionBW_dropdown->setCurrentIndex(settings.value("BANDWIDTH/ResolutionBW").toInt());
            if(settings.value("BANDWIDTH/SameRBW_VBW").toBool())
            {
                ui->same_RBW_VBW_checkBox->setChecked(true);
            }
            else
            {
                ui->same_RBW_VBW_checkBox->setChecked(false);
            }
            ui->videoBW_dropdown->setCurrentIndex(settings.value("BANDWIDTH/VideoBW").toInt());
        }

    }
    else
    {
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
        ui->attenuation_spinbox->setEnabled(false);
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


void scan_settings::on_frequency_dropdown_center_currentIndexChanged(int index)
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_spanfreq_spinbox_valueChanged(double arg1)
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_frequency_dropdown_span_currentIndexChanged(int index)
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_step_spinbox_valueChanged(int arg1)
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_referencelevel_spinbox_valueChanged(int arg1)
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_attenuation_spinbox_valueChanged(int arg1)
{
    ui->apply->setEnabled(true);
}

void scan_settings::on_leveloffset_spinbox_valueChanged(int arg1)
{
    ui->apply->setEnabled(true);
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
