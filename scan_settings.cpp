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
    ui->sweep_spinbox->setEnabled(false);
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
    center_span = true;
    start_stop = false;
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
    center_span = false;
    start_stop = true;
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
    if(center_span)
    {
        // Center frequency
        mystring = ":FREQuency:CENTer %1 %2\n";
        mystring = mystring.arg(QString::number(ui->center_freq_spinbox->value()), ui->frequency_dropdown_center->currentText());
        _socket_sa->write(mystring.toLocal8Bit());
        _socket_sa->waitForBytesWritten(1);
        mystring = "";

        // Span
        mystring = "::FREQuency:SPAN %1 %2\n";
        mystring = mystring.arg(QString::number(ui->spanfreq_spinbox->value()), ui->frequency_dropdown_span->currentText());
        _socket_sa->write(mystring.toLocal8Bit());
        _socket_sa->waitForBytesWritten(1);
        mystring = "";
    }
    else if(start_stop)
    {
        // Start frequency
        mystring = ":FREQuency:STARt %1 %2\n";
        mystring = mystring.arg(QString::number(ui->start_freq_spinbox->value()), ui->start_freq_dropdown->currentText());
        _socket_sa->write(mystring.toLocal8Bit());
        _socket_sa->waitForBytesWritten(1);
        qDebug() << mystring;
        mystring = "";

       // Stop frequency
        mystring = ":FREQuency:STOP %1 %2\n";
        mystring = mystring.arg(QString::number(ui->stop_freq_spinbox->value()), ui->stop_freq_dropdown->currentText());
        _socket_sa->write(mystring.toLocal8Bit());
        _socket_sa->waitForBytesWritten(1);
        qDebug() << mystring;
        mystring = "";
    }
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

void scan_settings::on_start_freq_dropdown_currentIndexChanged(int index)
{
    if(ui->stop_freq_dropdown->currentIndex() > index)
        ui->stop_freq_dropdown->setCurrentIndex(index);
}


void scan_settings::on_center_freq_spinbox_valueChanged(double arg1)
{
    if(arg1 >= sa_max_freq && ui->frequency_dropdown_center->currentText() == sa_max_freq_unit)
    {
        ui->center_freq_spinbox->setValue(sa_max_freq-0.1);
    }
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

void scan_settings::on_sweep_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0)
        ui->sweep_spinbox->setEnabled(false);
    else
        ui->sweep_spinbox->setEnabled(true);
}

void scan_settings::on_checkBox_2_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->videoBW_spinbox->setEnabled(true);
        ui->videoBW_dropdown->setEnabled(true);
    }
    else
    {
        ui->videoBW_spinbox->setEnabled(false);
        ui->videoBW_dropdown->setEnabled(false);
    }
}


