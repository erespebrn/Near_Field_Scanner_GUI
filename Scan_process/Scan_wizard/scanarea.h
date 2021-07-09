#ifndef SCANAREA_H
#define SCANAREA_H

#include <QDialog>

namespace Ui {
class scanArea;
}

class scanArea : public QDialog
{
    Q_OBJECT

public:
    explicit scanArea(QWidget *parent = nullptr);

    void add_preview_image(QImage&);
    ~scanArea();

private:
    Ui::scanArea *ui;
};

#endif // SCANAREA_H
