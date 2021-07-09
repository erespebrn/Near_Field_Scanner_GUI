#include "scanarea.h"
#include "ui_scanarea.h"

scanArea::scanArea(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::scanArea)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle("Selected area preview");
    this->move(1000,100);
}

void scanArea::add_preview_image(QImage &im)
{
    QImage resized = im.scaled(im.width()/2, im.height()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->preview_label->setPixmap(QPixmap::fromImage(resized));
    this->adjustSize();
}

scanArea::~scanArea()
{
    delete ui;
}
