#ifndef MAINSETTINGS_H
#define MAINSETTINGS_H

#include <QDialog>

namespace Ui {
class MainSettings;
}

class MainSettings : public QDialog
{
    Q_OBJECT

public:
    explicit MainSettings(QWidget *parent = nullptr);
    ~MainSettings();

private:
    Ui::MainSettings *ui;
};

#endif // MAINSETTINGS_H
