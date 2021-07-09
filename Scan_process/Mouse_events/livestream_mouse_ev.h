#ifndef LIVESTREAM_MOUSE_EV_H
#define LIVESTREAM_MOUSE_EV_H

#include <QLabel>
#include <QRubberBand>
#include <QMouseEvent>
#include <QMessageBox>

class LiveStream_mouse_ev : public QLabel
{
    Q_OBJECT

public:
    enum Selection_type{PCB_AT_POS_1, HEIGHT_MES_POINT, PCB_CORNER, PCB_REF_POINT, SCAN_AREA, NO_SELECTION};

    LiveStream_mouse_ev(QWidget *parent = nullptr);
    ~LiveStream_mouse_ev() override;

    inline void set_selection_type(Selection_type st) { sel = st; };

signals:
    void send_selected_pcb(QRect&);
    void send_selected_pcb_to_cv(int, int, int, int);

    void send_height_scan_point(QPoint);
    void send_height_scan_point_to_cv(int, int);

    void send_pcb_corner_point(QPoint);
    void send_pcb_corner_point_to_cv(int, int);

    void send_pcb_ref_point(QPoint);
    void send_pcb_ref_point_to_cv(int, int);

    void send_selected_scan_area(QRect&);

    void set_pcb_selected_flag(bool);
    void set_height_scan_selected_flag(bool);
    void set_pcb_corner_selected_flag(bool);
    void set_pcb_ref_point_selected_flag(bool);
    void set_scan_area_selected_flag(bool);

protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    QRubberBand *rubberBand{};
    QPoint origin{};
    QPoint mouse_pos{};
    Selection_type sel{NO_SELECTION};
    bool leftmouseevent{false};
    bool rubberSizeOK{false};
    bool stop_mouse{false};
};


#endif // LIVESTREAM_MOUSE_EV_H
