#ifndef QLABEL_MOUSEEVENT_H
#define QLABEL_MOUSEEVENT_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QRubberBand>
#include <QMouseEvent>

class QLabel_mouseEvent : public QLabel
{
    Q_OBJECT

public:
    QLabel_mouseEvent(QWidget *parent = nullptr);
    ~QLabel_mouseEvent();

signals:
    void sendMousePosition(QPoint&);
    void sendQrect(QRect&);

protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    QRubberBand *rubberBand;
    QPoint origin;
    QPoint mouse_pos;
    bool leftmouseevent {false};
    bool stop_mouse {false};
};

#endif // QLABEL_MOUSEEVENT_H
