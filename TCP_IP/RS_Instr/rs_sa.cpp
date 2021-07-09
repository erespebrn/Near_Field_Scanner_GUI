#include "rs_sa.h"

RS_sa::RS_sa() : RS_Instruments()
{
    this->rs_type = SA;
}

bool RS_sa::init(void)
{
    bool success{false};

    connect(this, &QTcpSocket::readyRead, this, &RS_sa::dataread);
    connect(this, &QTcpSocket::bytesWritten, this, &RS_sa::confirm_written_bytes);

    if(this->RS_Instruments::tcp_connect(sa_ip_address, sa_port)){
        //Init the device
        this->QTcpSocket::write("*RST\n");
        this->QTcpSocket::write("*ESE 1\n");
        this->QTcpSocket::write("SYST:DISP:UPD ON\n");
        this->QTcpSocket::write("FORM:DATA REAL,32\n");
        this->QTcpSocket::write("INIT:CONT OFF\n");

        success = true;
    }else{
        success = false;
    }
    return success;
}

void RS_sa::request_freq_values(void)
{
    log_file->add_log_line("SA freq request");
    this->QTcpSocket::write("TRAC:DATA:X? TRACE1\n");
}

void RS_sa::request_magnitudes()
{
    log_file->add_log_line("SA data request");
    this->QTcpSocket::write("DISP:TRAC1:MODE WRIT\n");
    this->QTcpSocket::write("DISP:TRAC1:MODE MAXH\n");
    this->QTcpSocket::write("INIT;*WAI\n");
    this->QTcpSocket::write("TRAC:DATA? TRACE1;*WAI\n");
}

void RS_sa::request_duration_time()
{
    duration_time = true;
    this->QTcpSocket::write("SWE:DUR?\n");
}

void RS_sa::send_command(QString &cmd)
{
    this->QTcpSocket::write(cmd.toLocal8Bit());
}

void RS_sa::send_command(const char *cmd)
{
    this->QTcpSocket::write(cmd);
}

void RS_sa::dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;

    b_data.append(this->QTcpSocket::readAll());

    if(duration_time){
        dur_time = 2*(no_of_sweeps*b_data.toDouble())+0.3;
        if(scan_started)
            log_file->add_log_line("SA duration time from ins: "+QString::number(dur_time)+"\n");
        duration_time = false;
        b_data.clear();
    }else{
        if(b_data.size() > sweep_points*4){
            if(time_for_amplitude){
                no[0] = b_data.at(1);
                digits = atoi(no);
                for(int i=0; i<digits; i++){
                    data.append(b_data.at(2+i));
                }
                data=data.left(digits);
                bytes = data.toUInt();
                log_file->add_log_line("Digits: "+QString::number(digits)+", Bytes: "+QString::number(bytes));
                if(bytes == sweep_points*4){
                    b_data.chop(1);
                    b_data=b_data.right(bytes);
                    const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                    for (int i=0; i<b_data.size()/4; ++i){
                        float d = *ptrFloat;
                        mag.push_back(d);
                        ptrFloat++;
                    }
                    temp2d.push_back(mag);
                    nearfield_plot->plot_maxhold();
                    nearfield_plot->add_data_point();
                    log_file->add_log_line("Current point: ("+QString::number(x_pos)+":"+QString::number(y_pos)+")");
                    log_file->add_log_line("Vector mag of size: "+QString::number(mag.size())+" pushed back to temp2d vector of size: "+QString::number(temp2d.size()));

                    if(x_pos == scan_columns){
                        data_tensor.push_back(temp2d);
                        temp2d.push_back(mag);
                        temp2d.clear();
                        log_file->add_log_line("Pushed back to data_tensor vector of size: "+QString::number(data_tensor.size()));
                    }
                    b_data.clear();
                    mag.clear();
                    digits = 0;
                    bytes = 0;
                }else{
                    log_file->add_log_line("Scan error! Magnitude data save error! Scan aborted!");
                    emit stop_scan();
                    QMessageBox::critical(nullptr, "Scan error!", "Magnitude data save error! Scan aborted!");
                }
            }else{
                no[0] = b_data.at(1);
                digits = atoi(no);
                for(int i=0; i<digits; i++){
                    data[i] = b_data.at(2+i);
                }
                bytes = atoi(data);
                log_file->add_log_line("Digits: "+QString::number(digits)+", Bytes: "+QString::number(bytes));

                if(bytes == sweep_points*4){
                    b_data.chop(1);
                    b_data=b_data.right(bytes);
                    const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                    for (int i=0; i<b_data.size()/4; ++i){
                        float d = *ptrFloat;
                        freq.push_back(d);
                        ptrFloat++;
                    }
                    this->save_data_to_file(RS_Instruments::FREQ);
                    log_file->add_log_line("Freq saved");
                    nearfield_plot->display_first_freq(freq.at(0), freq.at(freq.size()-1));
                    b_data.clear();
                    time_for_amplitude = true;
                    digits = 0;
                    bytes = 0;
                    return;
                }else{
                    log_file->add_log_line("Scan error! Frequency data save error! Scan aborted!");
                    emit stop_scan();
                    QMessageBox::critical(nullptr, "Scan error!", "Frequency data save error! Scan aborted!");
                }
            }
        }
    }
}

void RS_sa::save_data_to_file(Save sv)
{
    if(sv == FREQ){
        QString path = current_scan_datapath + "xaxis_data.bin";
        std::ofstream file(path.toLocal8Bit(), std::ios::binary);
        if(file.is_open()){
            file.write(reinterpret_cast<const char*>(&freq[0]), freq.size()*sizeof(float));
            file.close();
        }
    }else if(sv == YCOMP){
        QString path = current_scan_datapath+"y_comp_scan_data_tensor.bin";
        std::ofstream file;

        file.open(path.toLocal8Bit(), std::ios::binary);

        if(file){
            log_file->add_log_line("Magnitudes file does not exists. (Which is good!) Created file!");
            float sp = (float)sweep_points;
            float x_max = (float)scan_columns;
            float y_max = (float)scan_rows;

            float step_size_px = 1;
            float scan_width_px = 1;
            float scan_height_px = 1;

            float step_size_mm = step_size;
            float scan_width_mm = step_size_mm*scan_columns;
            float scan_height_mm = step_size_mm*scan_rows;

            file.write(reinterpret_cast<const char*>(&step_size_px), sizeof(step_size_px));
            file.write(reinterpret_cast<const char*>(&scan_width_px), sizeof(scan_width_px ));
            file.write(reinterpret_cast<const char*>(&scan_height_px), sizeof(scan_height_px));
            file.write(reinterpret_cast<const char*>(&step_size_mm), sizeof(step_size_mm));
            file.write(reinterpret_cast<const char*>(&scan_width_mm), sizeof(scan_width_mm));
            file.write(reinterpret_cast<const char*>(&scan_height_mm), sizeof(scan_height_mm));

            file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
            file.write(reinterpret_cast<const char*>(&x_max), sizeof(x_max));
            file.write(reinterpret_cast<const char*>(&y_max), sizeof(y_max));
        }else if(sv == XCOMP){
            log_file->add_log_line("File exists. No overwrite allowed!");
        }

        log_file->add_log_line("MAGNITUDES DATA SAVED TO FILE. SIZE OF THE VECTOR FOR Y COMP: "+QString::number(data_tensor.size()));
        data_tensor_mem = data_tensor;
        if(file.is_open()){
            for(auto &v : data_tensor){
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();
    }else{
        QString path = current_scan_datapath+"x_comp_scan_data_tensor.bin";
        std::ofstream file;

        file.open(path.toLocal8Bit(), std::ios::binary);

        if(file){
            log_file->add_log_line("Frequencies file does not exists. (Which is good!) Created file!");
            float sp = (float)sweep_points;
            float x_max = (float)scan_columns;
            float y_max = (float)scan_rows;

            float step_size_px = 1;
            float scan_width_px = 1;
            float scan_height_px = 1;

            float step_size_mm = 1;
            float scan_width_mm = 1;
            float scan_height_mm = 1;

            file.write(reinterpret_cast<const char*>(&step_size_px), sizeof(step_size_px));
            file.write(reinterpret_cast<const char*>(&scan_width_px), sizeof(scan_width_px ));
            file.write(reinterpret_cast<const char*>(&scan_height_px), sizeof(scan_height_px));
            file.write(reinterpret_cast<const char*>(&step_size_mm), sizeof(step_size_mm));
            file.write(reinterpret_cast<const char*>(&scan_width_mm), sizeof(scan_width_mm));
            file.write(reinterpret_cast<const char*>(&scan_height_mm), sizeof(scan_height_mm));

            file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
            file.write(reinterpret_cast<const char*>(&x_max), sizeof(x_max));
            file.write(reinterpret_cast<const char*>(&y_max), sizeof(y_max));
        }else{
            log_file->add_log_line("File exists. No overwrite allowed!");
        }

        log_file->add_log_line("FREQUENCIES DATA SAVED TO FILE. SIZE OF THE VECTOR FOR Y COMP: " + QString::number(data_tensor.size()));
        if(file.is_open()){
            for(auto &v : data_tensor){
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();
    }
}

void RS_sa::confirm_written_bytes(qint64 bytes)
{
    Q_UNUSED(bytes);
}
