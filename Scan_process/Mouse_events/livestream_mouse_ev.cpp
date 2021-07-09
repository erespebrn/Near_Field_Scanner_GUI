#include "Scan_process/Mouse_events/livestream_mouse_ev.h"
#include <QApplication>
#include <QDebug>

LiveStream_mouse_ev::LiveStream_mouse_ev(QWidget* parent) : QLabel(parent)
{}

LiveStream_mouse_ev::~LiveStream_mouse_ev()
{}

void LiveStream_mouse_ev::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton){
        if(sel == PCB_AT_POS_1 || sel == SCAN_AREA){

            origin = ev->pos();
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->setGeometry(QRect(origin, QSize(0,0)));
            leftmouseevent = true;
            rubberBand->show();
            QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

        }else if(sel == HEIGHT_MES_POINT){

            emit send_height_scan_point(ev->pos());
            emit send_height_scan_point_to_cv(ev->pos().x()-1, ev->pos().y()-1);
            emit set_height_scan_selected_flag(true);

            qDebug() << "New origin: " << ev->pos();

        }else if(sel == PCB_CORNER){

            emit send_pcb_corner_point(ev->pos());
            emit send_pcb_corner_point_to_cv(ev->pos().x()-1, ev->pos().y()-1);
            emit set_pcb_corner_selected_flag(true);

        }else if(sel == PCB_REF_POINT){

            emit send_pcb_ref_point(ev->pos());
            emit send_pcb_ref_point_to_cv(ev->pos().x(), ev->pos().y());
            emit set_pcb_ref_point_selected_flag(true);

        }else if(sel == NO_SELECTION){

            emit set_pcb_selected_flag(false);
            emit set_height_scan_selected_flag(false);
            emit set_pcb_ref_point_selected_flag(false);
        }
    }
}

void LiveStream_mouse_ev::mouseMoveEvent(QMouseEvent *ev)
{
    if(sel == PCB_AT_POS_1 || sel == SCAN_AREA){

        mouse_pos = ev->pos();

        if(mouse_pos.x() <= this->size().width() && mouse_pos.y() <= this->size().height()) {
            if(leftmouseevent){
                if(mouse_pos.x() > 0 && mouse_pos.y() > 0){

                    rubberBand->setGeometry(QRect(origin, ev->pos()));
                }
            }
        }
    }
}


void LiveStream_mouse_ev::mouseReleaseEvent(QMouseEvent *ev)
{
    if(sel == PCB_AT_POS_1 || sel == SCAN_AREA){
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        if(ev->button() == Qt::LeftButton) {
            if((ev->pos().x() - origin.x()) > 10 && (ev->pos().y() - origin.y()) > 10){

                QRect myRect(origin, ev->pos());
                if(sel == PCB_AT_POS_1){

                    QRect rect_comp = QRect(myRect.x()-1, myRect.y()-1, myRect.width()-1, myRect.height()-1);
                    emit send_selected_pcb(rect_comp);
                    emit send_selected_pcb_to_cv(myRect.x()-1, myRect.y()-1, myRect.width()-1, myRect.height()-1);
                    emit set_pcb_selected_flag(true);

                }else{

                    emit send_selected_scan_area(myRect);
                    emit set_scan_area_selected_flag(true);
                }
            }else{

                QMessageBox::warning(this, "Selected Area", "Selected Area is too small");
            }
        }
        leftmouseevent = false;
        rubberSizeOK = false;
        rubberBand->hide();
    }
}
