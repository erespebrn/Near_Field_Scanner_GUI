#include "scanwizard.h"
#include "ui_scanwizard.h"
#include <cmath>
ScanWizard::ScanWizard(QWidget *parent) :
    ui(new Ui::ScanWizard)
{
    ui->setupUi(this);
    par = parent;
}

ScanWizard::~ScanWizard()
{
    delete ui;
}

void ScanWizard::on_Next_button_clicked()
{
    switch(step++)
    {
        case(0):
            ui->label_2->setText("Place your PCB on the scannig mat as much parallel to the bottom edge as possible");
            ui->Next_button->setText("Next");
            break;
        case(1):
            ui->label_2->setText("Waiting for PCB...");
            ui->Next_button->setEnabled(false);

            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, &ScanWizard::check_for_pcb);
            timer->start(100);

            emit detect_pcb(true);
            break;
        case(2):
            ui->label_2->setText("Measuring PCB height...");
            emit detect_pcb(false);
            emit send_robot_to_origin(true);
            break;
        case(3):
            ui->label_2->setText("Using the Robot Manual control panel on the right side of the window, find the relevant camera position and take the picture of the PCB. "
                                 "Next, by using of mousie, select the area of scan");
            QPushButton * takepic_btn = new QPushButton;
            QPushButton * resetviev_btn = new QPushButton;
            takepic_btn->setText("Take Picture");
            resetviev_btn->setText("Reset view");
            takepic_btn->setStyleSheet("font: bold;");
            resetviev_btn->setStyleSheet("font: bold;");
            ui->horizontalLayout->addWidget(takepic_btn);
            ui->horizontalLayout->addWidget(resetviev_btn);
            connect(takepic_btn, SIGNAL(clicked()), par, SLOT(on_Take_img_button_clicked()));
            connect(resetviev_btn, SIGNAL(clicked()), par, SLOT(on_resetCamera_button_clicked()));
            break;


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
