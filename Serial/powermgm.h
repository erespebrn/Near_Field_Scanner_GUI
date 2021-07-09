#ifndef POWERMGM_H
#define POWERMGM_H

#include <QWidget>
#include <QtSerialPort>
#include <bitset>

namespace Ui {
class PowerMGM;
}

class PowerMGM : public QWidget
{
    Q_OBJECT

public:
    explicit PowerMGM(QWidget *parent = nullptr, QSerialPort *sp = nullptr);
    ~PowerMGM();

public slots:
    void readSerial(void);
    void on_sa_pushButton_clicked();
    void on_vna_pushButton_clicked();
    void on_robot_pushButton_clicked();
    void on_aux_pushButton_clicked();
    void on_eut_pushButton_clicked();

private slots:
    void on_ok_pushButton_clicked();

private:
    Ui::PowerMGM *ui;
    QSerialPort *arduino{};

};

#endif // POWERMGM_H
