#include "scanwizard.h"
#include "ui_scanwizard.h"
#include <cmath>
ScanWizard::ScanWizard(QWidget *parent) :
    ui(new Ui::ScanWizard)
{
    ui->setupUi(this);
    par = parent;

    ui->takepic_btn->setVisible(false);
    ui->resetview_btn->setVisible(false);
}

ScanWizard::~ScanWizard()
{
    delete ui;
}

void ScanWizard::on_Next_button_clicked()
{
    connect(ui->takepic_btn, SIGNAL(clicked()), par, SLOT(on_Take_img_button_clicked()));
    connect(ui->resetview_btn, SIGNAL(clicked()), par, SLOT(on_resetCamera_button_clicked()));
    ui->takepic_btn->setText("Take Picture");
    ui->resetview_btn->setText("Reset view");
    ui->takepic_btn->setStyleSheet("font: bold;");
    ui->resetview_btn->setStyleSheet("font: bold;");

    switch(step++)
    {
        case(0):
        {
            ui->label_2->setText("Place your PCB on the scannig mat as much parallel to the bottom edge as possible");
            ui->Next_button->setText("Next");
            break;
        }
        case(1):
        {
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
            ui->label_2->setText("Measuring PCB height...");
            emit detect_pcb(false);
            emit send_robot_to_origin(true);
            break;
        }
        case(3):
        {
            ui->label_2->setText("Using the Robot Manual control panel on the right side of the window, find the relevant camera position and take the picture of the PCB. "
                                 "Next, by using of mousie, select the area of scan");
            emit scan_area_origin_detect(true);
            ui->takepic_btn->setVisible(true);
            ui->resetview_btn->setVisible(true);
            break;
        }
        case(4):
        {
            ui->label_2->setText("Check if the robot is in the corner of the desired scanning area and press START to run the scan!");
            ui->takepic_btn->setVisible(false);
            ui->resetview_btn->setVisible(false);
            emit send_robot_to_origin(false);
            emit ui->resetview_btn->clicked();
            emit scan_area_origin_detect(false);
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
