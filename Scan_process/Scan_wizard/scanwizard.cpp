#include "scanwizard.h"
#include "ui_scanwizard.h"
#include <cmath>
#include <QThread>
#include <QDebug>
#include <QMessageBox>

ScanWizard::ScanWizard(QWidget *parent, Robot *r, VideoThread *v, Ui::scanner_gui *mui) : Scan_interface(parent, r, v, mui), ui(new Ui::ScanWizard)
{
    ui->setupUi(dynamic_cast<ScanWizard*>(this));

    //This ui
    ui->Next_button->setEnabled(true);
    ui->time_frame->setVisible(false);
    ui->comp_frame->setVisible(false);
    ui->back_button->setVisible(false);
    ui->instruments_settings->setVisible(false);
    ui->x_comp_checkbox->setVisible(false);
    ui->y_comp_checkbox->setVisible(false);
    ui->dut_name->setVisible(false);
    ui->dut_label->setVisible(false);
    ui->scan_height_label->setVisible(false);
    ui->scan_height_spinBox->setVisible(false);
    ui->skip_calibration_button->setVisible(false);
    ui->step_size_label->setVisible(false);
    ui->step_size_spinBox->setVisible(false);

    //Main window ui
    mw_ui->Start_scan_button->setEnabled(false);


    //Info from robot
    connect(r,  SIGNAL(scan_finished()),            this,   SLOT(scan_finished()));
    connect(r,  SIGNAL(scan_error()),               this,   SLOT(scan_error()));

    //Wizard image
    QImage *wiz_image = new QImage(":/img/images/nearfield_wizard.PNG");
    *wiz_image = wiz_image->convertToFormat(QImage::Format_ARGB32);
    QPainter p;
    p.begin(wiz_image);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(wiz_image->rect(), QColor(0, 0, 0, 80));
    p.end();
    ui->wizard_image->setPixmap(QPixmap::fromImage(*wiz_image));
    delete wiz_image;
}

ScanWizard::~ScanWizard()
{
    qDebug() << "Deleted wizard";
    delete ui;
}

void ScanWizard::scan_procedure()
{
    switch(step)
    {
        case(1):
        {
            //This ui
            ui->label->setText("DUT name");
            ui->title_label->setText("    Give a name of the Device Under Test");
            ui->info_label->setText("Type a name and press \"Next\". This name will be used to identify your scan results.");
            ui->Next_button->setText("Next");

            ui->dut_name->clear();
            ui->dut_name->setVisible(true);
            ui->dut_label->setVisible(true);
            ui->back_button->setVisible(false);
            ui->Next_button->setEnabled(false);


            this->resize(sizeHint());

            mw_ui->camera_focus_dial->setEnabled(false);
            mw_ui->camera_focus_dial->setValue(60);

            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::NO_SELECTION);
            this->Scan_interface::display_pcb_outline(false);
            this->Scan_interface::update_height_scan(10.0);
            break;
        }
        case(2):
        {
            //This ui
            ui->label->setText("Select the PCB");
            ui->title_label->setText("    Using the computer mouse, select the PCB on the camera live stream.");
            ui->info_label->setText("Left-lick on the PCB's top left corner. Next, "
                                    "without releasing, drag your mouse to the right bottom PCB's corner. Last, release the left-click.\n\n"
                                    "The selected PCB outline will be shown on the camera live stream.");

            ui->Next_button->setText("Next");
            ui->dut_name->setVisible(false);
            ui->dut_label->setVisible(false);
            ui->Next_button->setEnabled(false);
            ui->scan_height_label->setVisible(false);
            ui->scan_height_spinBox->setVisible(false);
            ui->back_button->setVisible(true);

            //Get and send the dut name to main window
            this->dut_name = ui->dut_name->text();

            //Timer to check whether the pcb has been selected
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::check_for_pcb_selected);
            timer->start(100);

            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::PCB_AT_POS_1);
            this->Scan_interface::display_scan_height_point(false);
            break;
        }
        case(3):
        {

            ui->title_label->setText("    The PCB selected!");
            QString mystring = "PCB selected info: \n\nSize of the PCB: %1x%2 mm\nArea: %3 mm\n\nPress \"Next\" to continue.";
            mystring = mystring.arg(QString::number(this->Scan_interface::get_pcb_size().size_mm.width()),
                                    QString::number(this->Scan_interface::get_pcb_size().size_mm.height()),
                                    QString::number(this->Scan_interface::get_pcb_size().size_mm.width()*this->Scan_interface::get_pcb_size().size_mm.height()));
            ui->info_label->setText(mystring);
            ui->Next_button->setEnabled(true);

            //Scan interface command
            this->pcb_sel = false;
            this->height_scan_point_sel = false;
            this->Scan_interface::display_scan_height_point(false);
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::NO_SELECTION);

            break;
        }
        case(4):
        {
            //This ui
            ui->label->setText("Measure the height");
            ui->title_label->setText("    Measure the height of the PCB above the mat.");
            ui->info_label->setText("Select a point on the camera live stream where the height measurement should be done"
                                    " by clicking on it. A circle with description should show up.\n\n"
                                    "The best choice for the height measurement is the highest point of the PCB"
                                    " or the part of the PCB where the near-field scan should be done.\n\n");

            ui->Next_button->setText("Next");
            ui->Next_button->setEnabled(false);

            //Timer to check whether the scan point has been selected or not
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::check_for_height_meas_point_selected);
            timer->start(100);

            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::HEIGHT_MES_POINT);
            this->Scan_interface::display_scan_height_point(true);
            break;
        }
        case(5):
        {
            //This ui
            ui->label->setText("Measure the height");
            ui->title_label->setText("    Measure the height of the PCB above the mat.");
            ui->info_label->setText("Measuring the PCB's height...");
            ui->Next_button->setVisible(false);
            ui->back_button->setVisible(false);
            ui->scan_height_label->setVisible(false);
            ui->scan_height_spinBox->setVisible(false);

            //Timer to check whether the pcb has been detected or not
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::check_for_height_measured);
            timer->start(100);

            //Scan interface command
            this->Scan_interface::disp_pcb_outline(false);
            this->Scan_interface::disp_height_scan_point(false);
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::NO_SELECTION);
            this->Scan_interface::measure_height();
            break;
        }
        case(6):
        {
            ui->label->setText("Height measured");
            ui->title_label->setText("    The height measurement process done");
            ui->info_label->setText("THe height measured. Press \"Next\" to continue! \n\n");
            ui->Next_button->setVisible(true);

            //Scan interface commands
            this->Scan_interface::goto_takepic2_position();
            this->Scan_interface::disp_height_scan_point(false);
            break;
        }
        case(7):
        {
            //This ui
            ui->label->setText("PCB corner");
            ui->title_label->setText("    Select the PCB's corner point.");
            ui->info_label->setText("Adjust the camera focus using knob on the right to achieve the maximum sharpeness of the picture. \n"
                                 "Next, by using the Robot's manual control panel on the right side of the window, find the relevant camera position.\n\n"
                                 "Select the PCB's top left corner. This point is necessary to identify PCB on the image and correlate it to the real world.\n\n");

            ui->Next_button->setEnabled(false);

            //Main window ui
            mw_ui->robotManualControl_frame->setStyleSheet("background-color: rgb(150,220,150)");
            mw_ui->camera_focus_dial->setEnabled(true);

            //Timer to check whether the pcb has been detected or not
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::check_for_pcb_corner_selected);
            timer->start(100);

            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::PCB_CORNER);
            this->Scan_interface::display_pcb_corner_point(true);

            break;
        }
        case(8):
        {
            //This ui
            ui->label->setText("PCB reference point");
            ui->title_label->setText("    Select the PCB reference point.");
            ui->info_label->setText("Select the reference point on your PCB by clicking on the camera live stream. This should be an easily recognizable point.\n\n"
                                    "Robot will move to that point. After that, the callibarion process will begin.");

            ui->Next_button->setEnabled(false);
            ui->back_button->setVisible(true);
            ui->Next_button->setText("Next");
            ui->skip_calibration_button->setVisible(false);

            //Main Window ui
            mw_ui->robotManualControl_frame->setEnabled(false);

            //Timer to check whether the pcb ref point has been selected or not
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::check_for_pcb_ref_point_selected);
            timer->start(100);

            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::PCB_REF_POINT);
            this->Scan_interface::display_pcb_corner_point(false);
            break;
        }
        case(9):
        {
            //This ui
            ui->label->setText("Calibration process");
            ui->title_label->setText("    Callibrate the robot position to reduce offset errors.");
            ui->info_label->setText("Check whether the Robot has moved right above the selected reference point or not. \n\n"
                                 "If yes, skip the calibration process by clicking \"Skip\". Otherwise, begin the callibraion process by clicking \"Calibrate\". \n\n");

            ui->Next_button->setText("Calibrate");
            ui->skip_calibration_button->setVisible(true);


            this->Scan_interface::set_takepic2_calib();
            this->Scan_interface::goto_pcb_ref_pos();
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::NO_SELECTION);
            this->Scan_interface::display_pcb_ref_point(false);
            break;
        }
        case(10):
        {
            //This ui
            ui->label->setText("Calibration process - Y component");
            ui->title_label->setText("    Callibrate the robot's position to reduce the offset errors on the Y component");
            ui->info_label->setText("By using the Robot's manual control, compensate the offset from the desired position in both, X and Y axes.\n\n"
                                    "To do this right, use only left/right arrows. The X-component offset will be calibrated in the next step. \n\n");

            ui->Next_button->setText("Next");
            ui->skip_calibration_button->setVisible(false);
            ui->back_button->setVisible(false);

            mw_ui->robotManualControl_frame->setEnabled(true);
            mw_ui->rotate_probe_button->setEnabled(false);

            //Scan interface commands
            this->Scan_interface::callib_x_setEnabled(true);

            break;
        }
        case(11):
        {
            //This ui
            ui->label->setText("Calibration process - X component");
            ui->title_label->setText("    Callibrate the robot's position to reduce the offset errors on the X component");
            ui->info_label->setText("By using the Robot's manual control, compensate the offset from the desired position in X and Y axes.\n\n"
                                    "To do this right, use only up/down arrows. The Y-component offsest has already been calibrated in the previous step. \n\n");

            //Scan interface commands
            this->Scan_interface::callib_x_setEnabled(false);
            this->Scan_interface::callib_y_setEnabled(true);

            break;
        }
        case(12):
        {
            ui->label->setText("Calibration done");
            ui->title_label->setText("    The calibraion process finished sucessfully.");
            ui->info_label->setText("Pres \"Next\" to continue the scan setup. \n\n");

            mw_ui->rotate_probe_button->setEnabled(true);

            //Scan interface command
            this->Scan_interface::callib_y_setEnabled(false);
            this->Scan_interface::goto_takepic2_calib();
            break;
        }
        case(13):
        {
            //This ui
            ui->label->setText("Scan area");
            ui->title_label->setText("    Using the mouse, select the area for the scan.");
            ui->info_label->setText("Left-click on the top left corner of your desired scan area.\n\n"
                                    "Next, without releasing, drag your mouse to the right bottom corner of the scan are and release the left-click.\n\n"
                                    "The selected area will be displayed in a seperate window.");
            ui->instruments_settings->setVisible(false);
            ui->Next_button->setEnabled(true);
            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::SCAN_AREA);
            break;
        }
        case(14):
        {
            //This ui
            ui->label->setText("Instruments settings");
            ui->title_label->setText("    Select the measurement instruments settings.");
            ui->info_label->setText("The robot now moved to the corner of the desired scan area.\n\n"
                                  "Please now set the measurement instruments setting by pressing the button below.");

            ui->Next_button->setEnabled(false);
            ui->comp_frame->setVisible(false);
            ui->x_comp_checkbox->setVisible(false);
            ui->y_comp_checkbox->setVisible(false);
            ui->step_size_label->setVisible(false);
            ui->step_size_spinBox->setVisible(false);
            ui->back_button->setVisible(true);
            ui->instruments_settings->setVisible(true);

            //Main window ui
            mw_ui->robotManualControl_frame->setStyleSheet("");

            //Timer to check whether the instrument object has been created or not
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::check_for_instruments_created);
            timer->start(100);

            //Scan interface command
            this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::NO_SELECTION);
            if(!this->Scan_interface::scan_area_preview->isHidden())
                this->Scan_interface::scan_area_preview->hide();
            this->Scan_interface::goto_scan_origin();
            break;
        }
        case(15):
        {
            //This ui
            ui->label->setText("Field component and step size");
            ui->title_label->setText("    Select the step size and required field component(s).");
            ui->info_label->setText("Using the checkboxes below, check the field components which you want to measure.\n"
                                    "Next, select the step size and height for your scan using the fields below:");
            ui->Next_button->setText("Next");
            ui->scan_height_label->setVisible(true);
            ui->scan_height_spinBox->setVisible(true);
            ui->comp_frame->setVisible(true);
            ui->x_comp_checkbox->setVisible(true);
            ui->y_comp_checkbox->setVisible(true);
            ui->step_size_label->setVisible(true);
            ui->step_size_spinBox->setVisible(true);
            ui->instruments_settings->setVisible(false);
            break;
        }
        case(16):
        {
            //This ui

            ui->label->setText("Run the scan");
            ui->title_label->setText("    Last step before the scan.");
            ui->info_label->setText("If everything was set correcly, Press \"Start Scan\" to start the scanning process!");
            ui->Next_button->setText("Start Scan");

            ui->scan_height_label->setVisible(false);
            ui->scan_height_spinBox->setVisible(false);
            ui->step_size_label->setVisible(true);
            ui->step_size_spinBox->setVisible(true);
            ui->comp_frame->setVisible(false);
            ui->step_size_label->setVisible(false);
            ui->step_size_spinBox->setVisible(false);

            //Scan interface commands
            this->Scan_interface::assign_scan_field_components(ui->x_comp_checkbox->isChecked(), ui->y_comp_checkbox->isChecked());
            this->Scan_interface::event_logfile_init(this->Scan_interface::dut_name);
            this->Scan_interface::step_size = ui->step_size_spinBox->value();
            this->Scan_interface::update_height_scan(ui->scan_height_spinBox->value());
            break;
        }
        case(17):
        {
            //This ui
            ui->label->setText("Scan in progress..");
            ui->title_label->setText("");
            ui->info_label->setText("Scan is now in progress. Pres Stop to abort.");
            ui->Next_button->setText("Stop");
            ui->time_frame->setVisible(true);
            ui->back_button->setVisible(false);

            //Main window ui
            mw_ui->robotManualControl_frame->setStyleSheet("");

            //Scan interface commands
            this->Scan_interface::start_scan();

            //Estimated time setup and timer to count down
            this->est_time_init();
            timer = std::unique_ptr<QTimer> (new QTimer);
            connect(timer.get(), &QTimer::timeout, this, &ScanWizard::update_time);
            timer->start(1000);

            break;
        }
        case(18):
        {
            //This ui
            ui->label->setText("Scan aborted");
            ui->title_label->setText("");
            ui->info_label->setText("Scan aborted. Moving back to the homeposition.");
            ui->Cancel_button->setText("Close");
            ui->Next_button->setVisible(false);
            ui->time_frame->setVisible(false);

            //Scan interface commands
            this->Scan_interface::stop_scan(Scan_interface::SCAN_ABORTED);

            break;
        }
    }
}

void ScanWizard::on_Next_button_clicked()
{
    this->step++;
    this->scan_procedure();
}

void ScanWizard::on_back_button_clicked()
{
    if(step == 4)
        this->step = 2;
    else if(step == 9 || step == 14){
        this->Scan_interface::goto_takepic2_calib();
        this->step--;
    }
    else
        this->step--;

    this->video->display_pcb_outline(false);
    this->scan_procedure();
}

void ScanWizard::check_for_pcb_selected()
{
    if(this->Scan_interface::pcb_sel){
        timer->stop();
        this->step = 3;
        this->scan_procedure();
    }
}

void ScanWizard::check_for_height_measured()
{
    if(this->Scan_interface::scan_height_measured){
        timer->stop();
        this->step = 6;
        this->scan_procedure();
    }
}

void ScanWizard::check_for_height_meas_point_selected()
{
    if(this->Scan_interface::height_scan_point_sel){
        timer->stop();
        ui->Next_button->setEnabled(true);
    }
}

void ScanWizard::check_for_pcb_corner_selected()
{
    if(this->Scan_interface::pcb_corner_point_sel){
        timer->stop();
        ui->Next_button->setEnabled(true);
    }
}

void ScanWizard::check_for_pcb_ref_point_selected()
{
    if(this->Scan_interface::pcb_ref_point_sel){
        timer->stop();
        ui->Next_button->setEnabled(true);
    }
}


void ScanWizard::check_for_instruments_created()
{
    if(this->Scan_interface::ins_created){
        timer->stop();
        ui->Next_button->setEnabled(true);
    }
}

void ScanWizard::est_time_init()
{
    time = this->Scan_interface::get_estimated_time();
    s = time%60;
    m = ((time-s)/60)%60;
    h = round((time/60/60));

    ui->seconds->setText(QString::number(s));
    ui->minutes->setText(QString::number(m));
    ui->hours->setText(QString::number(h));
}

void ScanWizard::update_time()
{
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

void ScanWizard::scan_finished()
{
    ui->label->setText("Scan finished");
    ui->info_label->setText("Scanning process finished!");
    ui->Cancel_button->setText("Close");

    ui->Next_button->setVisible(false);
    ui->time_frame->setVisible(false);

    this->Scan_interface::stop_scan(Scan_stop_reason::SCAN_FINISHED);
}


void ScanWizard::on_dut_name_textChanged(const QString &arg1)
{
    if(arg1.length() >= 2)
        ui->Next_button->setEnabled(true);
}

void ScanWizard::on_Cancel_button_clicked()
{
    this->close();
}


void ScanWizard::scan_error()
{
    this->Scan_interface::stop_scan(Scan_stop_reason::SCAN_ERROR);
}

void ScanWizard::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Scan Wizard" ,"Are you sure you want to close?\n All progress will be lost.",
                                                              QMessageBox::No | QMessageBox::Yes);
    if(resBtn != QMessageBox::Yes){
        event->ignore();
    }else{
        mw_ui->robotManualControl_frame->setStyleSheet("");
        mw_ui->stepsize_xy->setStyleSheet("");
        mw_ui->stepsize_z->setStyleSheet("");
        mw_ui->robotManualControl_frame->setEnabled(true);
        mw_ui->rotate_probe_button->setEnabled(true);


        this->Scan_interface::mw_ui->liveStream->set_selection_type(LiveStream_mouse_ev::NO_SELECTION);
        this->Scan_interface::display_pcb_outline(false);
        this->Scan_interface::display_pcb_ref_point(false);
        this->Scan_interface::display_scan_height_point(false);
        this->Scan_interface::goto_home_pos();

        mw_ui->Start_scan_button->setEnabled(true);
        event->accept();
    }
}

void ScanWizard::on_instruments_settings_clicked()
{
    scan_settings scan_settings(nullptr, this->Scan_interface::create_rs_instrument());

    scan_settings.setStyleSheet("");
    scan_settings.setWindowFlags(scan_settings.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    scan_settings.setModal(true);
    scan_settings.setFixedSize(scan_settings.width(),scan_settings.height());
    scan_settings.exec();
}


