#include "scanwizard.h"
#include "ui_scanwizard.h"
#include <cmath>
#include <QThread>
#include <QDebug>
#include <QMessageBox>

ScanWizard::ScanWizard(QWidget *parent, Robot *r, DUT_size *p, DUT_size *s) :
    ui(new Ui::ScanWizard)
{
    ui->setupUi(this);
    par = parent;
    robot = r;
    pcb = p;
    scan_area = s;
    ui->takepic_btn->setVisible(false);
    ui->resetview_btn->setVisible(false);
    ui->Next_button->setEnabled(false);
    ui->time_frame->setVisible(false);
    ui->comp_frame->setVisible(false);
    ui->x_comp_checkbox->setVisible(false);
    ui->y_comp_checkbox->setVisible(false);

    connect(ui->takepic_btn, SIGNAL(clicked()), par, SLOT(take_img()));
    connect(ui->resetview_btn, SIGNAL(clicked()), par, SLOT(reset_camera()));
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
            dut_name = ui->dut_name->text();
            emit send_dut_name(dut_name);
            ui->dut_name->setVisible(false);
            ui->dut_label->setVisible(false);
            ui->label->setText("Place PCB");
            ui->title_label->setText("    Place your PCB on the scannig mat in order to be detected by the system.");
            ui->info_label->setText("* Important is to place is as much parallel to the bottom table edge as possible.\n\n"
                                    "* Adjust the camera focus using knob on the right to achieve maximum sharpeness of the picture\n\n"
                                    "* Make sure the PCB color is contrasting to the mat. If not, use the white sheet of paper to place PCB on.\n\n");
            ui->Next_button->setText("Next");
            this->resize(sizeHint());
            break;
        }
        case(1):
        {
            ui->label->setText("Detecting PCB");
            ui->title_label->setText("    Waiting for PCB...");
            ui->info_label->setText("* Reference window has opened. The PCB should be illuminated as a white rectangle on the black background.\n\n"
                                    "* In case that PCB cannot be detected, try to place it on the white piece of paper sheet with rectangular shape.");
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
            ui->title_label->setText("    Measure the height of the PCB above the mat.");
            ui->info_label->setText("* Select the point on the picture where the height measurement should be done"
                                    " by clicking on it. A circle with description should show up.\n\n"
                                    "* The best choice for the height measurement is the highest point of the PCB\n\n"
                                    "  or the part of the PCB where the near-field scan should be done\n\n"
                                    "* Select the scan height using the highlighted spin-box");
            ui->Next_button->setText("Next");
            ui->Next_button->setEnabled(false);
            emit allow_emit_pos(true);
            break;
        }
        case(3):
        {
            ui->label->setText("PCB height");
            ui->info_label->setText("Measuring PCB height...");
            ui->Next_button->setVisible(false);
            robot->goto_meas_height(pcb->scan_height_point.x(), pcb->scan_height_point.y());
            emit detect_pcb(false);
            emit allow_emit_pos(false);
            break;
        }
        case(4):
        {
            ui->label->setText("Scan area");
            ui->title_label->setText("    Select the scan area.");
            ui->info_label->setText("* Adjust the camera focus using knob on the right to achieve maximum sharpeness of the picture\n\n"
                                 "* Using the Robot Manual control panel on the right side of the window, find the relevant camera position.\n\n"
                                 "* Make sure that the 'Corner' circle is still present and take a picture.\n\n"
                                 "* Next, by using of mouse, select the area of scan");

            robot->goto_takepic2_pos(pcb->size_mm.width()/2, pcb->size_mm.width()/2);
            robot->ask_for_camera_height();

            emit set_scan_settings(step);
            emit scan_area_origin_detect(true);
            ui->takepic_btn->setVisible(true);
            ui->resetview_btn->setVisible(true);
            break;
        }
        case(5):
        {
            emit scan_area_origin_detect(false);
            ui->takepic_btn->setVisible(false);
            ui->resetview_btn->setVisible(false);
            emit ui->resetview_btn->clicked();           
            emit set_scan_settings(step);
            emit kill_cv_window();

            robot->goto_origin(pcb->corner.x()-scan_area->corner.x(), pcb->corner.y()-scan_area->corner.y());

            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, &ScanWizard::check_for_instruments_created);
            timer->start(100);

            ui->Next_button->setEnabled(false);
            ui->label->setText("Scan settings");
            ui->title_label->setText("    Select the measurement instruments settings.");
            ui->info_label->setText("Robot now moved to a corner of desired scan area.\n\n "
                                 "* If needed, compensate the position of the robot manually\n\n"
                                 "* Please now set the measurement instruments setting in the Scan Settings (highlighted in green) menu on the right side option bar!");
            break;
        }
        case(6):
        {
            emit scan_area_origin_detect(false);
            emit ui->resetview_btn->clicked();
            emit set_scan_settings(step);
            ui->comp_frame->setVisible(true);
            ui->x_comp_checkbox->setVisible(true);
            ui->y_comp_checkbox->setVisible(true);
            ui->label->setText("Step size/scan height");
            ui->title_label->setText("    Select the step size and required field component(s).");
            ui->info_label->setText("* Set the scanning step size on the right side option bar (highlighted in green).\n"
                                    "* Select the field components which you would like to measure.");
            break;
        }
        case(7):
        {
            robot->x_comp_sel = ui->x_comp_checkbox->isChecked();
            robot->y_comp_sel = ui->y_comp_checkbox->isChecked();
            robot->ins->log_file->log_file_init(dut_name);

            ui->comp_frame->setVisible(false);
            ui->label->setText("Run the scan");
            ui->title_label->setText("    Last step before the scan.");
            emit set_scan_settings(step);
            ui->info_label->setText("If everything was set correcly, Press 'Start Scan' to start the scanning process!");
            ui->Next_button->setText("Start Scan");
            break;
        }
        case(8):
        {
            emit set_scan_settings(step);

            robot->set_scan_parameters(scan_area->step_size);
            robot->start_scan();

            ui->time_frame->setVisible(true);

            est_time_init();

            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, &ScanWizard::update_time);
            timer->start(1000);

            ui->label->setText("Scan in progress..");
            ui->title_label->setText("");
            ui->info_label->setText("Scan is now in progress. Pres Stop to abort.");
            ui->Next_button->setText("Stop");
            break;
        }
        case(9):
        {
            robot->stop_scan();
            robot->ins->log_file->add_log_line("SCAN STOPPED BY A USER!");
            emit set_scan_settings(step);
            emit scan_stopped();

            ui->label->setText("Scan aborted");
            ui->title_label->setText("");
            ui->info_label->setText("Scan aborted. Moving back to the homeposition.");
            ui->Cancel_button->setText("Close");
            ui->Next_button->setVisible(false);
            ui->time_frame->setVisible(false);
            scan_done = true;
            break;
        }
    }
}

void ScanWizard::est_time_init()
{
    time = robot->total_time;
    s = time%60;
    m = ((time-s)/60)%60;
    h = round((time/60/60));

    ui->seconds->setText(QString::number(s));
    ui->minutes->setText(QString::number(m));
    ui->hours->setText(QString::number(h));
}

void ScanWizard::update_time()
{
    //qDebug() << "Timer here. Time left: " << time;
    time--;
    if(time>=0){
        s--;
        if(s<0){
            s=59;
            m--;
            if(m<0){
                m=59;
                h--;
                if(h<0){
                    h=0;
                }
            }
        }
        ui->seconds->setText(QString::number(s));
        ui->minutes->setText(QString::number(m));
        ui->hours->setText(QString::number(h));
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
        ui->title_label->setText("    The system detected your PCB!");
        QString mystring = "PCB detected info: \n\nSize of the PCB: %1x%2 mm\nArea: %3 mm\nDistance from the origin: %4 mm\n\nPlease make sure that the values make sense!";
        mystring = mystring.arg(QString::number(pcb->size_mm.width()), QString::number(pcb->size_mm.height()), QString::number(pcb->size_mm.width()*pcb->size_mm.height()),
                                QString::number(int(sqrt(pow(pcb->corner.x(),2)+pow(pcb->corner.y(),2)))));
        ui->info_label->setText(mystring);
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

    if(!scan_done)
        emit scan_stopped();

    this->deleteLater();
    this->hide();
}

void ScanWizard::height_measure_finished()
{
    ui->label->setText("Height measured");
    ui->info_label->setText("Height measured! \n\nFinding the spot!");
    ui->Next_button->setVisible(true);
    ui->Next_button->clicked();
}

void ScanWizard::scan_finished()
{
    robot->ins->log_file->add_log_line("SCAN FINISHED!");
    ui->label->setText("Scan finished");
    ui->info_label->setText("Scanning process finished!");
    ui->Cancel_button->setText("Close");

    ui->Next_button->setVisible(false);
    ui->time_frame->setVisible(false);
    scan_done = true;
    emit scan_stopped();
}

void ScanWizard::scan_aborted()
{
    emit set_scan_settings(step);
    robot->ins->log_file->add_log_line("SCAN STOPPED DUE TO ERROR!");
    ui->label->setText("Scan aborted");
    ui->info_label->setText("Scan aborted. Moving back to the homeposition.");
    ui->Cancel_button->setText("Close");
    ui->Next_button->setVisible(false);
    ui->time_frame->setVisible(false);
}

void ScanWizard::check_for_instruments_created()
{
    if(ins_creat){
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

void ScanWizard::on_dut_name_textChanged(const QString &arg1)
{
    if(arg1.length() >= 2)
        ui->Next_button->setEnabled(true);
}

void ScanWizard::height_meas_point_selected()
{
    ui->Next_button->setEnabled(true);
}

void ScanWizard::scan_error()
{
    ui->Next_button->clicked();
    QMessageBox::critical(this, "Scan error", "An error occurred during the scan. Please restart the scan!");
}
