#include "dut_size.h"

QPoint DUT_size::scan_pcb_corner = QPoint(0,0);
QRect DUT_size::croppedOrigin = QRect(0,0,0,0);
uint32_t DUT_size::real_height = 0;
uint32_t DUT_size::camera_distance_2 = 100000;

DUT_size::DUT_size()
{}


void DUT_size::cv_getcoord(bool scan, int o_x, int o_y, int pcb_x, int pcb_y, int pcb_w, int pcb_h)
{
    origin = QPoint(o_x, o_y);

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float width_cropped = (camera_distance*pcb_w*sensor_width/(focal_lenght*1280));
    float height_cropped = (camera_distance*pcb_h*sensor_height/(focal_lenght*960));

    float x_dist_px = pcb_x - origin.x();
    float y_dist_px = pcb_y - origin.y();

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280));
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960));

    if(!scan)
    {
        corner = QPoint(int(round(x_dist_mm)), int(round(y_dist_mm)));
        size_mm = QRect(corner.x(), corner.y(), int(round(width_cropped)), int(round(height_cropped)));
        emit send_coord_to_wizard(corner, size_mm);
    }
    else
    {
        scan_pcb_corner = QPoint(o_x, o_y);
    }
}

void DUT_size::send_area_request()
{
    int w_px = (size_mm.width()*focal_lenght*1280)/(camera_distance_2*sensor_width);
    int h_px = (size_mm.height()*focal_lenght*960)/(camera_distance_2*sensor_height);

    emit send_area_to_videothread(w_px*h_px);
}

void DUT_size::receive_scanheight_point(int x, int y)
{
    float x_dist_px = x - origin.x();
    float y_dist_px = y - origin.y();

    float x_dist_mm = (camera_distance*x_dist_px*sensor_width/(focal_lenght*1280));
    float y_dist_mm = (camera_distance*y_dist_px*sensor_height/(focal_lenght*960));

    scan_height_point = QPoint(x_dist_mm+18, y_dist_mm);
}

void DUT_size::receive_cropped_area(QRect &rect)
{
    croppedOrigin = rect;

    //Height and width of cropped image (marked using mouse) can be computed using the following equations
    float width_cropped = ((float)camera_distance_2*(float)rect.width()*sensor_width/(focal_lenght*1280));
    float height_cropped = ((float)camera_distance_2*(float)rect.height()*sensor_height/(focal_lenght*960));

    float x_dist_px = scan_pcb_corner.x() - croppedOrigin.x();
    float y_dist_px = scan_pcb_corner.y() - croppedOrigin.y();

    float x_dist_mm = ((float)camera_distance_2*x_dist_px*sensor_width/(focal_lenght*1280));
    float y_dist_mm = ((float)camera_distance_2*y_dist_px*sensor_height/(focal_lenght*960));

    size_px = QRect(x_dist_px, y_dist_px, rect.width(), rect.height());
    corner = QPoint(int(round(x_dist_mm)), int(round(y_dist_mm)));
    size_mm = QRect(corner.x(), corner.y(), int(round(width_cropped)), int(round(height_cropped)));
}
