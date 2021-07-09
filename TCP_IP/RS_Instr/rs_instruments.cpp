#include "rs_instruments.h"
#include <qdebug.h>
#include <fstream>
#include <QMessageBox>


RS_Instruments::RS_Instruments() : Tcp_device()
{
    qDebug() << "RS instruments created";
}

RS_Instruments::~RS_Instruments()
{
    qDebug() << "RS Instrument destroyed";
}

void RS_Instruments::init_scan()
{
    nearfield_plot->init_plot(scan_columns, scan_rows);
    nearfield_plot->assign_data_tensors(&freq, &mag, &temp2d, &data_tensor, &data_tensor_mem, &phase, &temp2d_phase, &data_tensor_phase, &data_tensor_mem_phase);
    nearfield_plot->maxhold_init((size_t)sweep_points);
    this->reset_saved_data();
    this->request_freq_values();
}

void RS_Instruments::reset_saved_data()
{
    data_tensor.clear();
    data_tensor_phase.clear();
    temp2d_phase.clear();
    temp2d.clear();
    mag.clear();
    phase.clear();
    mag_received = false;
    nearfield_plot->set_current_scan_comp(NF_plot::YCOMP);
}
