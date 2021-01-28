#include "mainsettings.h"
#include "ui_mainsettings.h"


MainSettings::MainSettings(QWidget *parent, Robot *r) :
    QDialog(parent),
    ui(new Ui::MainSettings)
{
    ui->setupUi(this);
    this->setWindowTitle("Main Settings");
    robot = r;
    mainsettings = new QSettings(QCoreApplication::applicationDirPath() + "/settings/mainsettings.ini", QSettings::IniFormat);
    this->read_previous_values();
}

MainSettings::~MainSettings()
{
    delete mainsettings;
    delete ui;
}

void MainSettings::on_robot_ref00_x_offset_valueChanged(double arg1)
{
    ref00_x = arg1;
    mainsettings->setValue("ref00_x", arg1);
    mainsettings->sync();
}

void MainSettings::on_robot_ref00_y_offset_valueChanged(double arg1)
{
    ref00_y = arg1;
    mainsettings->setValue("ref00_y", arg1);
    mainsettings->sync();
}

void MainSettings::on_height_meas_x_offset_valueChanged(double arg1)
{
    height_off_x = arg1;
    mainsettings->setValue("height_off_x", arg1);
    mainsettings->sync();
}

void MainSettings::on_height_meas_y_offset_valueChanged(double arg1)
{
    height_off_y = arg1;
    mainsettings->setValue("height_off_y", arg1);
    mainsettings->sync();
}

void MainSettings::on_start_scan_x_offset_valueChanged(double arg1)
{
    start_scan_off_x = arg1;
    mainsettings->setValue("start_scan_off_x", arg1);
    mainsettings->sync();
}

void MainSettings::on_start_scan_y_offset_valueChanged(double arg1)
{
    start_scan_off_y = arg1;
    mainsettings->setValue("start_scan_off_y", arg1);
    mainsettings->sync();
}

void MainSettings::read_previous_values()
{
    ref00_x = mainsettings->value("ref00_x").toDouble();
    ref00_y = mainsettings->value("ref00_y").toDouble();
    height_off_x = mainsettings->value("height_off_x").toDouble();
    height_off_y = mainsettings->value("height_off_y").toDouble();
    start_scan_off_x = mainsettings->value("start_scan_off_x").toDouble();
    start_scan_off_y = mainsettings->value("start_scan_off_y").toDouble();

    ui->robot_ref00_x_offset->setValue(ref00_x);
    ui->robot_ref00_y_offset->setValue(ref00_y);
    ui->height_meas_x_offset->setValue(height_off_x);
    ui->height_meas_y_offset->setValue(height_off_y);
    ui->start_scan_x_offset->setValue(start_scan_off_x);
    ui->start_scan_y_offset->setValue(start_scan_off_y);
    assign_offsets();
}

void MainSettings::assign_offsets()
{
    robot->height_off_x = height_off_x;
    robot->height_off_y = height_off_y;
    robot->start_scan_off_x = start_scan_off_x;
    robot->start_scan_off_y = start_scan_off_y;
    emit send_ref00_offsets(ref00_x, ref00_y);
}

void MainSettings::init_logfiles()
{
    Event_log *log = new Event_log;
    model = new QStringListModel;
    model->setStringList(log->load_logfile_list());
    ui->logfiles_list->setModel(model);
    ui->logfiles_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    delete log;
}

void MainSettings::on_OK_button_clicked()
{
    this->assign_offsets();
    this->hide();
}

void MainSettings::on_Cancel_button_clicked()
{
    this->hide();
}

void MainSettings::on_apply_button_clicked()
{
    this->assign_offsets();
}

void MainSettings::on_Settings_tab_widget_currentChanged(int index)
{
    if(index==1){
        this->init_logfiles();
        ui->apply_button->setVisible(false);
        ui->OK_button->setVisible(false);
    }else{
        ui->apply_button->setVisible(true);
        ui->OK_button->setVisible(true);
        delete model;
    }
}

void MainSettings::on_open_logfile_clicked()
{
    QModelIndex index = ui->logfiles_list->currentIndex();
    QString filename = index.data(Qt::DisplayRole).toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/event_logs/" + filename));
}
