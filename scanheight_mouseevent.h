#ifndef SCANHEIGHT_MOUSEEVENT_H
#define SCANHEIGHT_MOUSEEVENT_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QRubberBand>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>

class scanheight_mouseevent : public QLabel
{
    Q_OBJECT

signals:
    void sendPos(int, int);
    void height_meas_point_selected();

public:
    scanheight_mouseevent(QWidget *parent = nullptr);

public slots:
    void allow_emit(bool);

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    //void mouseReleaseEvent(QMouseEvent *ev) override;
private:
    bool allow_to_emit = false;
};

#endif // SCANHEIGHT_MOUSEEVENT_H
