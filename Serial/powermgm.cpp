#include "powermgm.h"
#include "ui_powermgm.h"

PowerMGM::PowerMGM(QWidget *parent, QSerialPort *sp) : QWidget(parent), ui(new Ui::PowerMGM), arduino{sp}
{
    ui->setupUi(this);

    connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));

    if(arduino->isOpen()){
        QByteArray d;
        d.push_back(20);
        arduino->write(d);
        ui->serial_indicator->setText("Connected");
    }else{
        ui->serial_indicator->setText("NOT connected");
    }

    ui->control_frame->setEnabled(arduino->isOpen());
}

PowerMGM::~PowerMGM()
{
    qDebug() << "Delete powerMGM";
    delete ui;
}

void PowerMGM::readSerial()
{
    bool ok{};
    int data = arduino->readAll().toHex().toInt(&ok, 16);

    std::bitset<5> st(data);

    qDebug() << "data";

    st[0] ? ui->sa_indicator->setPixmap(QPixmap(":/img/images/led_on.png"))      : ui->sa_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));
    st[1] ? ui->vna_indicator->setPixmap(QPixmap(":/img/images/led_on.png"))     : ui->vna_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));
    st[2] ? ui->robot_indicator->setPixmap(QPixmap(":/img/images/led_on.png"))   : ui->robot_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));
    st[3] ? ui->aux_indicator->setPixmap(QPixmap(":/img/images/led_on.png"))     : ui->aux_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));
    st[4] ? ui->eut_indicator->setPixmap(QPixmap(":/img/images/led_on.png"))     : ui->eut_indicator->setPixmap(QPixmap(":/img/images/led_off.png"));

    st[0] ? ui->sa_pushButton->setText("OFF")        : ui->sa_pushButton->setText("ON");
    st[1] ? ui->vna_pushButton->setText("OFF")       : ui->vna_pushButton->setText("ON");
    st[2] ? ui->robot_pushButton->setText("OFF")     : ui->robot_pushButton->setText("ON");
    st[3] ? ui->aux_pushButton->setText("OFF")       : ui->aux_pushButton->setText("ON");
    st[4] ? ui->eut_pushButton->setText("OFF")       : ui->eut_pushButton->setText("ON");
}

void PowerMGM::on_sa_pushButton_clicked()
{
    QByteArray d;
    d.push_back(10);
    arduino->write(d);
}

void PowerMGM::on_vna_pushButton_clicked()
{
    QByteArray d;
    d.push_back(11);
    arduino->write(d);
}

void PowerMGM::on_robot_pushButton_clicked()
{
    QByteArray d;
    d.push_back(12);
    arduino->write(d);
}

void PowerMGM::on_aux_pushButton_clicked()
{
    QByteArray d;
    d.push_back(13);
    arduino->write(d);
}

void PowerMGM::on_eut_pushButton_clicked()
{
    QByteArray d;
    d.push_back(14);
    arduino->write(d);
}

void PowerMGM::on_ok_pushButton_clicked()
{
    this->close();
}
