#include "scan_interface.h"

Scan_interface::Scan_interface(QWidget *parent, Robot *r, VideoThread *v, Ui::scanner_gui *mui) : QWidget(nullptr), mw_ui{mui}, video{v}, par{parent}, robot{r}
{
    //Scan interface and mouse events signals/slots
    connect(this,   SIGNAL(display_scan_height_point(bool)),    video,  SLOT(display_scan_height_point(bool)));
    connect(this,   SIGNAL(display_pcb_outline(bool)),          video,  SLOT(display_pcb_outline(bool)));
    connect(this,   SIGNAL(display_pcb_corner_point(bool)),     video,  SLOT(display_pcb_corner_point(bool)));
    connect(this,   SIGNAL(display_pcb_ref_point(bool)),        video,  SLOT(display_pcb_ref_point(bool)));

    //Scan interface and robot signals/slots
    connect(r,  SIGNAL(height_measured()),          this,   SLOT(height_measured()));

    //LiveStream - this signals/slots
    connect(mw_ui->liveStream, SIGNAL(set_pcb_selected_flag(bool)),             this, SLOT(set_pcb_selected_flag(bool)));
    connect(mw_ui->liveStream, SIGNAL(set_height_scan_selected_flag(bool)),     this, SLOT(set_height_scan_selected_flag(bool)));
    connect(mw_ui->liveStream, SIGNAL(set_pcb_ref_point_selected_flag(bool)),   this, SLOT(set_pcb_ref_point_selected_flag(bool)));
    connect(mw_ui->liveStream, SIGNAL(set_pcb_corner_selected_flag(bool)),      this, SLOT(set_pcb_corner_selected_flag(bool)));
    connect(mw_ui->liveStream, SIGNAL(set_scan_area_selected_flag(bool)),       this, SLOT(set_scan_area_selected_flag(bool)));

    connect(mw_ui->liveStream, SIGNAL(send_selected_scan_area(QRect&)),         this, SLOT(processCroppedImage(QRect&)));

    //LiveStream - video signals/slots
    connect(mw_ui->liveStream, SIGNAL(send_selected_pcb_to_cv(int, int, int, int)), video, SLOT(receive_selected_pcb(int, int, int, int)));
    connect(mw_ui->liveStream, SIGNAL(send_height_scan_point_to_cv(int, int)),      video, SLOT(receive_selected_scan_height_point(int, int)));
    connect(mw_ui->liveStream, SIGNAL(send_pcb_corner_point_to_cv(int, int)),       video, SLOT(receive_selected_pcb_corner_point(int, int)));
    connect(mw_ui->liveStream, SIGNAL(send_pcb_ref_point_to_cv(int, int)),          video, SLOT(receive_selected_pcb_ref_point(int, int)));

    connect(this, SIGNAL(restart_instr_detector(void)), par, SLOT(restart_instr_detector()));

    this->create_dut_sizes();

    ins = new RS_sa;
    nearfield_plot = new NF_plot_sa;
    log_file = new Event_log(Event_log::events);
    scan_area_preview = new scanArea(nullptr);
}

Scan_interface::~Scan_interface()
{
    delete pcb;
    delete scan_area;
    delete scan_area_preview;
    delete ins;
    delete log_file;
    delete nearfield_plot;
}

void Scan_interface::measure_height() { robot->goto_meas_height(pcb->scan_height_point.x()-7, pcb->scan_height_point.y()-17); }

void Scan_interface::goto_scan_origin() { robot->goto_origin(pcb->corner.x() + scan_area->corner.x()-10, pcb->corner.y() + scan_area->corner.y()-10); }

void Scan_interface::goto_home_pos() { robot->goto_home_pos(); }

void Scan_interface::goto_pcb_ref_pos() { robot->goto_origin(pcb->corner.x() + scan_area->pcb_ref_point.x()-10, pcb->corner.y() + scan_area->pcb_ref_point.y()-10); }

void Scan_interface::callib_x_setEnabled(bool en) { robot->calib_x_enable(en); }

void Scan_interface::callib_y_setEnabled(bool en) { robot->calib_y_enable(en); }

void Scan_interface::goto_takepic2_position() { robot->goto_takepic2_pos(pcb->size_mm.width()/2, pcb->size_mm.width()/2); robot->ask_for_camera_height(); }

void Scan_interface::set_takepic2_calib() { robot->set_takepic2_calib(); }

void Scan_interface::goto_takepic2_calib() { robot->goto_takepic2_calib(); robot->ask_for_camera_height(); }


RS_Instruments *Scan_interface::create_rs_instrument()
{
    RS_sa *sa;
    RS_vna *vna;

    bool wo_vna = false;
    bool wo_sa = false;

    if(vna_connected_bool || sa_connected_bool){
        if(sa_connected_bool && vna_connected_bool){
            QMessageBox *msgBox = new QMessageBox;
            msgBox->setMinimumSize(400,200);
            msgBox->setWindowTitle("Measurement instrument");
            msgBox->setText("Both, SA and VNA connected!");
            msgBox->setInformativeText("Which one do you want to use?");
            QAbstractButton *SA = msgBox->addButton(("SA"), QMessageBox::YesRole);
            QAbstractButton *VNA = msgBox->addButton(("VNA"), QMessageBox::NoRole);
            msgBox->setIcon(QMessageBox::Question);
            msgBox->exec();

            if(msgBox->clickedButton() == SA){
                sa = new RS_sa;
                nearfield_plot = new NF_plot_sa;
                if(sa->init()){
                    wo_vna = true;
                }else{
                    QMessageBox::critical(par, "SA connection error!", "Error during the connection atempt!");
                    return nullptr;
                }
                delete msgBox;
            }else if(msgBox->clickedButton() == VNA){
                vna = new RS_vna;
                nearfield_plot = new NF_plot_vna;
                if(vna->init()){
                    wo_sa = true;
                }else{
                    QMessageBox::critical(par, "VNA connection error!", "Error during the connection atempt!");
                    return nullptr;
                }
                delete msgBox;
            }

        }else{
            if(sa_connected_bool){
                sa = new RS_sa;
                nearfield_plot = new NF_plot_sa;
                if(sa->init()){
                    wo_vna = true;
                }else{
                    QMessageBox::critical(par, "SA connection error!", "Error during the connection atempt!");
                    return nullptr;
                }
            }
            if(vna_connected_bool){
                vna = new RS_vna;
                nearfield_plot = new NF_plot_vna;
                if(vna->init()){
                    wo_sa = true;
                }else{
                    QMessageBox::critical(par, "VNA connection error!", "Error during the connection atempt!");
                    return nullptr;
                }
            }
        }

        if(wo_vna){
            ins = sa;
        }else if(wo_sa){
            ins = vna;
        }

        ins->set_step_size(mw_ui->stepsize_xy->value());
        robot->assign_rs_instrument(ins);

        ins_created = true;
        return ins;
    }else{
        QMessageBox::critical(par, "No instrument", "No measurement instrument connected!");
        return nullptr;
    }
}


void Scan_interface::create_dut_sizes()
{
    //*** Detected PCB and scan area
    pcb = new DUT_size;
    scan_area = new DUT_size;

    connect(mw_ui->liveStream, SIGNAL(send_selected_pcb(QRect&)),        pcb,        SLOT(receive_selected_pcb(QRect&)));
    connect(mw_ui->liveStream, SIGNAL(send_height_scan_point(QPoint)),   pcb,        SLOT(receive_scanheight_point(QPoint)));
    connect(mw_ui->liveStream, SIGNAL(send_pcb_corner_point(QPoint)),    scan_area,  SLOT(receive_pcb_corner_point(QPoint)));
    connect(mw_ui->liveStream, SIGNAL(send_pcb_ref_point(QPoint)),       scan_area,  SLOT(receive_pcb_ref_point(QPoint)));
    connect(mw_ui->liveStream, SIGNAL(send_selected_scan_area(QRect&)),  scan_area,  SLOT(receive_scan_area(QRect&)));

    robot->assign_duts(pcb, scan_area);
    robot->allow_emit_area(true);
}

void Scan_interface::processCroppedImage(QRect &rect)
{
    const QPixmap* pixmap = mw_ui->liveStream->pixmap();
    QImage image( pixmap->toImage() );
    QImage cropped = image.copy(rect);
    QImage scaledImage = cropped.scaled(mw_ui->liveStream->size().width()/2, mw_ui->liveStream->size().height()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    croppedImg = scaledImage;

    scan_area_preview->add_preview_image(croppedImg);
    scan_area_preview->show();
}

void Scan_interface::assign_scan_field_components(bool x, bool y)
{
    robot->set_scan_components(x,y);
    nearfield_plot->set_scan_components(x,y);
}

void Scan_interface::event_logfile_init(const QString &dut_name)
{
   log_file->log_file_init(dut_name);
}

void Scan_interface::start_scan()
{

    current_scan_datapath = datapath + "SCAN_" + dut_name + "__" + QDate::currentDate().toString("dd_MM_yyyy") + "__" + QTime::currentTime().toString("hh_mm_ss") + "/";

    if(!QDir(current_scan_datapath).exists())
        QDir().mkdir(current_scan_datapath);

    croppedImg.save(current_scan_datapath+"cropped_image.png", "PNG",100);
    nearfield_plot->add_bg_image(&croppedImg);
    log_file->log_file_init(dut_name);
    nearfield_plot->set_dut_name(dut_name);

    this->nearfield_plot->allow_exports(false);
    robot->set_scan_parameters(this->step_size);
    ins->set_datapath(current_scan_datapath);
    ins->set_nearfield_plot(nearfield_plot);
    ins->set_eventlog_file(log_file);
    ins->init_scan();
    robot->start_scan();
    nearfield_plot->show();
}

void Scan_interface::stop_scan(Scan_interface::Scan_stop_reason reason)
{
    if(reason == SCAN_ABORTED){

        //Stop robot and send to the home position
        robot->stop_scan();
        robot->goto_home_pos();
        robot->allow_emit_area(true);

        //Add a position in the log file
        log_file->add_log_line("SCAN ABORTED BY A USER!");

    }else if(reason == SCAN_ERROR){

        //Stop robot and send to the home position
        robot->stop_scan();
        robot->goto_home_pos();
        robot->allow_emit_area(true);

        //Add a position in the log file
        log_file->add_log_line("SCAN ABORTED BY A USER!");

    }else if(reason == SCAN_FINISHED){
        //If the scan has finished successfully, the robot will go to the home pos

        //Add a position in the log file
        log_file->add_log_line("SCAN FINISHED SUCCESSFULLY!");

        //Allow plot exports
        this->nearfield_plot->allow_exports(true);
    }
    //log_file->save_to_new_logfile();
    emit restart_instr_detector();
}


