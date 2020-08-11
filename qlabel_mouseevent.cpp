#include "qlabel_mouseevent.h"

QLabel_mouseEvent::QLabel_mouseEvent(QWidget* parent) : QLabel(parent)
{}

QLabel_mouseEvent::~QLabel_mouseEvent()
{}

void QLabel_mouseEvent::mouseMoveEvent(QMouseEvent *ev)
{
    mouse_pos = ev->pos();
    emit sendMousePosition(mouse_pos);
    if(mouse_pos.x() <= this->size().width() && mouse_pos.y() <= this->size().height())
    {
        if(leftmouseevent)
        {
            if(mouse_pos.x() > 0 && mouse_pos.y() > 0)
            {
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
        rubberBand->setGeometry(QRect(origin, QSize(1,1)));
        leftmouseevent = true;
        rubberBand->show();
    }
}

void QLabel_mouseEvent::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        if((ev->pos().x() - origin.x()) > 10 && (ev->pos().y() - origin.y()) > 10)
        {
            QRect myRect(origin, ev->pos());
            emit sendQrect(myRect);
        }
        else
        {
            QMessageBox::warning(this, "Selected Area", "Selected Area is too small");
        }
    }
    leftmouseevent = false;
    rubberSizeOK = false;
    rubberBand->hide();
}
