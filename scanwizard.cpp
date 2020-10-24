#include "scanwizard.h"
#include "ui_scanwizard.h"
#include <cmath>
#include <QThread>
#include <QDebug>
ScanWizard::ScanWizard(QWidget *parent) :
    ui(new Ui::ScanWizard)
{
    ui->setupUi(this);
    par = parent;

    ui->takepic_btn->setVisible(false);
    ui->resetview_btn->setVisible(false);

    connect(ui->takepic_btn, SIGNAL(clicked()), par, SLOT(Take_img_button_clicked()));
    connect(ui->resetview_btn, SIGNAL(clicked()), par, SLOT(resetCamera_button_clicked()));
    connect(ui->Cancel_button, SIGNAL(clicked()), par, SLOT(on_home_button_clicked()));
}

ScanWizard::~ScanWizard()
{
    emit set_scan_settings(10);
    delete ui;
}

void ScanWizard::on_Next_button_clicked()
{
    ui->takepic_btn->setText("Take Picture");
    ui->resetview_btn->setText("Reset view");
    ui->takepic_btn->setStyleSheet("font: bold;");
    ui->resetview_btn->setStyleSheet("font: bold;");

    switch(step++)
    {
        case(0):
        {
            ui->label->setText("Place PCB");
            ui->label_2->setText("Place your PCB on the scannig mat.\n\n"
                                 "* Important is to place is as much parallel to the bottom table edge as possible.");
            ui->Next_button->setText("Next");
            break;
        }
        case(1):
        {
            ui->label->setText("Detecting PCB");
            ui->label_2->setText("Waiting for PCB...");
            ui->Next_button->setEnabled(false);

            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, &ScanWizard::check_for_pcb);
            timer->start(100);

            emit detect_pcb(true);
            break;
        }
        case(2):
        {
            ui->label->setText("PCB height");
            ui->label_2->setText("Measuring PCB height...");
            ui->Next_button->setVisible(false);
            emit detect_pcb(false);
            emit send_robot_to_origin(true);
            break;
        }
        case(3):
        {
            ui->label->setText("Scan area");
            ui->label_2->setText("* Using the Robot Manual control panel on the right side of the window, find the relevant camera position.\n\n"
                                 "* Make sure that the 'Corner' circle is still present and take a picture.\n\n"
                                 "* If the picture is blured, move the robot higher using Z+\n\n"
                                 "* Next, by using of mousie, select the area of scan");
            emit set_scan_settings(step);
            emit scan_area_origin_detect(true);
            ui->takepic_btn->setVisible(true);
            ui->resetview_btn->setVisible(true);
            break;
        }
        case(4):
        {
            ui->label->setText("Scan settings");
            ui->label_2->setText("Robot now moved to a corner of desired scan area.\n\n "
                                 "* Please now set the measurement instruments setting in the Scan Settings (highlighted in green) menu on the right side option bar!");
            ui->takepic_btn->setVisible(false);
            ui->resetview_btn->setVisible(false);
            emit send_robot_to_origin(false);
            emit ui->resetview_btn->clicked();
            emit scan_area_origin_detect(false);
            emit set_scan_settings(step);
            break;
        }
        case(5):
        {
            ui->label->setText("Step size/scan height");
            emit set_scan_settings(step);
            ui->label_2->setText("Set the scanning step size and scanning height on the right side option bar (highlighted in green)");
            break;
        }
        case(6):
        {
            ui->label->setText("Run the scan");
            emit set_scan_settings(step);
            ui->label_2->setText("If everything was set correcly, Press 'Start Scan' to start the scanning process!");
            ui->Next_button->setText("Start Scan");
            break;
        }
        case(7):
        {
            ui->label->setText("Scan in progress..");
            emit set_scan_settings(step);
            emit run_scan(true);
            ui->label_2->setText("Scan is now in progress. Pres Stop to abort.");
            ui->Next_button->setText("Stop");
            break;
        }
        case(8):
        {
            emit run_scan(false);
            ui->label->setText("Scan aborted");
            emit set_scan_settings(step);
            ui->label_2->setText("Scan aborted. Moving back to the homeposition.");
            ui->Next_button->setVisible(false);
            ui->Cancel_button->setText("Close");
            break;
        }
    }
}



void ScanWizard::pcb_found()
{
    _pcb_found = true;
}

void ScanWizard::check_for_pcb()
{
    if(_pcb_found)
    {
        timer->stop();
        ui->label->setText("PCB detected!");
        QString mystring = "PCB detected\n\nSize of the PCB: %1x%2 mm\nArea: %3 mm\nDistance from the origin: %4 mm";
        mystring = mystring.arg(QString::number(pcb_size.width()), QString::number(pcb_size.height()), QString::number(pcb_size.width()*pcb_size.height()),
                                QString::number(int(sqrt(pow(pcb_corner.x(),2)+pow(pcb_corner.y(),2)))));
        ui->label_2->setText(mystring);
        ui->Next_button->setEnabled(true);
    }
}

void ScanWizard::take_coord(QPoint corner, QRect size)
{
    pcb_corner = corner;
    pcb_size = size;
}

void ScanWizard::on_Cancel_button_clicked()
{
    emit scan_area_origin_detect(false);
    emit detect_pcb(false);
    emit ui->resetview_btn->clicked();
    this->deleteLater();
    this->hide();
}

void ScanWizard::height_measure_finished()
{
    ui->label->setText("Height measured");
    ui->label_2->setText("Height measured!");
    ui->Next_button->setVisible(true);
    emit ui->Next_button->clicked();
}

void ScanWizard::scan_finished()
{
    ui->label->setText("Scan finished");
    ui->label_2->setText("Scanning process finished!");
    ui->Next_button->setVisible(false);
    ui->Cancel_button->setText("Close");
}
