#include "qlabel_mouseevent.h"

QLabel_mouseEvent::QLabel_mouseEvent(QWidget* parent) : QLabel(parent)
{}

QLabel_mouseEvent::~QLabel_mouseEvent()
{}

void QLabel_mouseEvent::mouseMoveEvent(QMouseEvent *ev)
{
    mouse_pos = ev->pos();
    if(mouse_pos.x() <= this->size().width() && mouse_pos.y() <= this->size().height())
    {
        if(mouse_pos.x() > 0 && mouse_pos.y() > 0)
        {
            if(leftmouseevent)
            {
                emit sendMousePosition(mouse_pos);
                rubberBand->setGeometry(QRect(origin, ev->pos()).normalized());
            }
        }
    }
}

void QLabel_mouseEvent::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        origin = ev->pos();
        //if (!rubberBand)
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        //rubberBand->setGeometry(QRect(origin, QSize(20,10)));
        leftmouseevent = true;
        rubberBand->show();
    }
}

void QLabel_mouseEvent::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        QRect myRect(origin, ev->pos());
        rubberBand->hide();
        leftmouseevent = false;
        emit sendQrect(myRect);
    }
}
