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
    emit allow_emit_pos(false);
    //delete timer;
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
                                 "* Important is to place is as much parallel to the bottom table edge as possible.\n\n"
                                 "* Adjust the camera focus using knob on the right to achieve maximum sharpeness of the picture"
                                 "* Make sure the PCB color is contrasting to the mat. If not, use the white sheet of paper to place PCB on.\n\n");
            ui->Next_button->setText("Next");
            break;
        }
        case(1):
        {
            ui->label->setText("Detecting PCB");
            ui->label_2->setText("Waiting for PCB...\n\n"
                                 "* Reference window has opened. The PCB should be illuminated as a white rectangle on the black background.\n\n"
                                 "* In case that PCB cannot be detected, try to place it on the white piece of paper sheet with rectangle shape.");
            ui->Next_button->setEnabled(false);

            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, &ScanWizard::check_for_pcb);
            timer->start(100);

            emit detect_pcb(true);
            break;
        }
        case(2):
        {
            emit set_scan_settings(step);
            ui->label->setText("Measure the height");
            ui->label_2->setText("Measure the height of the PCB above the mat: \n\n"
                                 "* Select the point on the picture where the height measurement should be done"
                                 " by clicking on it. A circle with description should show up.\n\n"
                                 "* The best choice for the height measurement is the highest point of the PCB\n\n"
                                 "  or the part of the PCB where the near-field scan should be done\n\n"
                                 "* Select the scan height using the highlighted spin-box");
            ui->Next_button->setText("Next");
            emit allow_emit_pos(true);
            break;
        }
        case(3):
        {
            ui->label->setText("PCB height");
            ui->label_2->setText("Measuring PCB height...");
            ui->Next_button->setVisible(false);
            emit detect_pcb(false);
            emit allow_emit_pos(false);
            emit send_robot_to_origin(true);
            break;
        }
        case(4):
        {
            ui->label->setText("Scan area");
            ui->label_2->setText("* Adjust the camera focus using knob on the right to achieve maximum sharpeness of the picture\n\n"
                                 "* Using the Robot Manual control panel on the right side of the window, find the relevant camera position.\n\n"
                                 "* Make sure that the 'Corner' circle is still present and take a picture.\n\n"
                                 "* If the picture is blured, move the robot higher using Z+\n\n"
                                 "* Next, by using of mouse, select the area of scan");
            emit set_scan_settings(step);
            emit send_for_2nd_takepic();
            emit scan_area_origin_detect(true);
            ui->takepic_btn->setVisible(true);
            ui->resetview_btn->setVisible(true);
            emit ask_for_cam_height();
            break;
        }
        case(5):
        {
            emit scan_area_origin_detect(false);
            ui->takepic_btn->setVisible(false);
            ui->resetview_btn->setVisible(false);
            emit ui->resetview_btn->clicked();
            emit send_robot_to_origin(false);
            emit set_scan_settings(step);

            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, &ScanWizard::check_for_instruments_created);
            timer->start(100);

            ui->Next_button->setEnabled(false);
            ui->label->setText("Scan settings");
            ui->label_2->setText("Robot now moved to a corner of desired scan area.\n\n "
                                 "* If needed, compensate the position of the robot manually\n\n"
                                 "* Please now set the measurement instruments setting in the Scan Settings (highlighted in green) menu on the right side option bar!");
            break;
        }
        case(6):
        {
            emit scan_area_origin_detect(false);
            emit ui->resetview_btn->clicked();
            emit set_scan_settings(step);
            ui->label->setText("Step size/scan height");
            ui->label_2->setText("Set the scanning step size and scanning height on the right side option bar (highlighted in green)");
            break;
        }
        case(7):
        {
            ui->label->setText("Run the scan");
            emit set_scan_settings(step);
            ui->label_2->setText("If everything was set correcly, Press 'Start Scan' to start the scanning process!");
            ui->Next_button->setText("Start Scan");
            break;
        }
        case(8):
        {
            ui->label->setText("Scan in progress..");
            emit set_scan_settings(step);
            emit run_scan(true);
            ui->label_2->setText("Scan is now in progress. Pres Stop to abort.");
            ui->Next_button->setText("Stop");
            break;
        }
        case(9):
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
    emit allow_emit_pos(false);

    this->deleteLater();
    this->hide();
}

void ScanWizard::height_measure_finished()
{
    ui->label->setText("Height measured");
    ui->label_2->setText("Height measured! \n\nFinding the spot!");    
    ui->Next_button->setVisible(true);
    ui->Next_button->clicked();
}

void ScanWizard::scan_finished()
{
    ui->label->setText("Scan finished");
    ui->label_2->setText("Scanning process finished!");
    ui->Next_button->setVisible(false);
    ui->Cancel_button->setText("Close");
}

void ScanWizard::check_for_instruments_created()
{
    if(ins_creat)
    {
        timer->stop();
        timer->deleteLater();
        ui->Next_button->setEnabled(true);
        emit set_scan_settings(10);
    }
}

void ScanWizard::inst_created()
{
    ins_creat = true;
}

