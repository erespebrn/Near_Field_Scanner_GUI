#include "dut_size.h"

QPoint DUT_size::scan_pcb_corner = QPoint(0,0);
QRect DUT_size::croppedOrigin = QRect(0,0,0,0);
uint32_t DUT_size::real_height = 0;
float DUT_size::camera_distance_2 = 100000.0;
const float DUT_size::camera_distance{864.0};

DUT_size::DUT_size()
{
    qDebug() << "DUT size created";
}

DUT_size::~DUT_size()
{
    qDebug() << "DUT size destroyed";
}

// For auto PCB recognition
//void DUT_size::cv_getcoord(bool scan, int o_x, int o_y, int pcb_x, int pcb_y, int pcb_w, int pcb_h)
//{
//    origin = QPoint(o_x, o_y);

//    //Height and width of cropped image (marked using mouse) can be computed using the following equations
//    float width_cropped = (camera_distance*pcb_w*sensor_width/(focal_lenght*1280));
//    float height_cropped = (camera_distance*pcb_h*sensor_height/(focal_lenght*960));

//    float x_dist_px = pcb_x - origin.x();
//    float y_dist_px = pcb_y - origin.y();

//    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280));
//    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960));

//    if(!scan){
//        corner = QPoint(int(round(x_dist_mm)), int(round(y_dist_mm)));
//        size_mm = QRect(corner.x(), corner.y(), int(round(width_cropped)), int(round(height_cropped)));
//    }else{
//        this->scan_pcb_corner = QPoint(o_x, o_y);
//    }
//}

void DUT_size::receive_selected_pcb(QRect &sel_pcb)
{
    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float width_cropped = (camera_distance*sel_pcb.width()*sensor_width/(focal_lenght*1280.0));
    float height_cropped = (camera_distance*sel_pcb.height()*sensor_height/(focal_lenght*960.0));

    float x_dist_px = sel_pcb.x() - robot_origin_fixed.x();
    float y_dist_px = sel_pcb.y() - robot_origin_fixed.y();

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280.0));
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960.0));

    corner = QPointF(x_dist_mm, y_dist_mm);
    size_mm = QRect(corner.x(), corner.y(), width_cropped, height_cropped-1);
}

void DUT_size::receive_scanheight_point(QPoint p)
{
    float x_dist_px = p.x() - robot_origin_fixed.x();
    float y_dist_px = p.y() - robot_origin_fixed.y();

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280.0));
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960.0));

    scan_height_point = QPointF(x_dist_mm, y_dist_mm);

    qDebug() << "Scan height point: " << scan_height_point;
}

void DUT_size::receive_pcb_ref_point(QPoint p)
{
    float x_dist_px = p.x() - pcb_corner.x();
    float y_dist_px = p.y() - pcb_corner.y();

    float x_dist_mm = (camera_distance_2*x_dist_px*sensor_width/(focal_lenght*1280.0));
    float y_dist_mm = (camera_distance_2*y_dist_px*sensor_height/(focal_lenght*960.0));

    pcb_ref_point = QPointF(x_dist_mm, y_dist_mm);

    qDebug() << "Camera distance: " << camera_distance_2 << "PCB REF: " << pcb_ref_point;
}

void DUT_size::receive_scan_area(QRect &rect)
{
    croppedOrigin = rect;

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float width_cropped = (camera_distance_2*(float)croppedOrigin.width()*sensor_width/(focal_lenght*1280.0));
    float height_cropped = (camera_distance_2*(float)croppedOrigin.height()*sensor_height/(focal_lenght*960.0));

    float x_dist_px = croppedOrigin.x() - pcb_corner.x();
    float y_dist_px = croppedOrigin.y() - pcb_corner.y();

    float x_dist_mm = (camera_distance_2*x_dist_px*sensor_width/(focal_lenght*1280.0));
    float y_dist_mm = (camera_distance_2*y_dist_px*sensor_height/(focal_lenght*960.0));

    size_px = QRect(x_dist_px, y_dist_px, rect.width(), rect.height());
    corner = QPointF(x_dist_mm, y_dist_mm);
    size_mm = QRect(corner.x(), corner.y(), width_cropped, height_cropped);

    qDebug() << "Camera distance: " << camera_distance_2 << "SCAN AREA CORNER: " << corner;

}

