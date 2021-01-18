#include "mainsettings.h"
#include "ui_mainsettings.h"

MainSettings::MainSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainSettings)
{
    ui->setupUi(this);
}

MainSettings::~MainSettings()
{
    delete ui;
}
