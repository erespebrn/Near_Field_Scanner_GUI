#include "scanheight_mouseevent.h"

scanheight_mouseevent::scanheight_mouseevent(QWidget* parent) : QLabel(parent)
{}

void scanheight_mouseevent::allow_emit(bool b)
{
    allow_to_emit = b;
}

void scanheight_mouseevent::mousePressEvent(QMouseEvent *ev)
{
    if(allow_to_emit)
    {
        emit sendPos(ev->x(), ev->y());
        emit height_meas_point_selected();
    }
}
