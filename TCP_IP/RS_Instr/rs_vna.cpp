#include "rs_vna.h"


RS_vna::RS_vna() : RS_Instruments{}
{
    this->rs_type = VNA;
}

bool RS_vna::init(void)
{
    bool success{false};

    connect(this, &QTcpSocket::readyRead, this, &RS_vna::dataread);
    connect(this, &QTcpSocket::bytesWritten, this, &RS_vna::confirm_written_bytes);

    if(this->RS_Instruments::tcp_connect(vna_ip_addr, vna_port)){

        //Init the device
        this->QTcpSocket::write("*RST\n");
        this->QTcpSocket::write("SYST:DISP:BAR:STO ON\n");
        this->QTcpSocket::write("SYST:DISP:UPD ON\n");
        this->QTcpSocket::write("FORM:DATA REAL,32\n");
        this->QTcpSocket::write("FORM:DEXP:SOUR FDAT\n");
        this->QTcpSocket::write("INIT:CONT OFF\n");

        // Delete all existing traces in VNA
        this->QTcpSocket::write("CALC:PAR:DEl:ALL\n");

        success = true;
    }else{
        success = false;
    }
    return success;
}

void RS_vna::request_freq_values()
{
    log_file->add_log_line("VNA freq request");
    vna_stimulus_values = true;

    if(mode == Mode_A)
        this->QTcpSocket::write("TRAC:DATA:STIM? CH1DATA;*WAI\n");
    else
        this->QTcpSocket::write("TRAC:DATA:STIM? CH3DATA;*WAI\n");
}

void RS_vna::request_magnitudes()
{
    qDebug() << "VNA data request";

    if(mode == Mode_A){
        this->QTcpSocket::write("INIT1;*WAI\n");
    }else if(mode == Mode_B){
        this->QTcpSocket::write("INIT1;*WAI\n");
        this->QTcpSocket::write("INIT2;*WAI\n");
        this->QTcpSocket::write("INIT3;*WAI\n");
    }

    this->request_mag();
    timer = std::unique_ptr<QTimer> (new QTimer);
    connect(timer.get(), &QTimer::timeout, this, &RS_vna::check_for_mag_received);
    timer->start(100);
}

void RS_vna::check_for_mag_received()
{
    if(mag_received) this->request_phase() ;
}

void RS_vna::request_mag()
{
    if(mode == Mode_A){
        this->QTcpSocket::write("CALC1:PAR:SEL 'Mag'\n");
        this->QTcpSocket::write("TRAC? CH1DATA;*WAI\n");
    }else if(mode == Mode_B){
        this->QTcpSocket::write("CALC3:PAR:SEL 'Mag'\n");
        this->QTcpSocket::write("TRAC? CH3DATA;*WAI\n");
    }
}

void RS_vna::request_phase()
{
    if(mode == Mode_A){
        this->QTcpSocket::write("CALC1:PAR:SEL 'Ph'\n");
        this->QTcpSocket::write("TRAC? CH1DATA;*WAI\n");
    }else if(mode == Mode_B){
        this->QTcpSocket::write("CALC3:PAR:SEL 'Ph'\n");
        this->QTcpSocket::write("TRAC? CH3DATA;*WAI\n");
    }
}

void RS_vna::request_duration_time()
{
    duration_time = true;
    this->QTcpSocket::write("SWE:TIME?\n");
}

void RS_vna::send_command(QString &cmd)
{
    this->QTcpSocket::write(cmd.toLocal8Bit());
}

void RS_vna::send_command(const char *cmd)
{
    this->QTcpSocket::write(cmd);
}

void RS_vna::dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;


    b_data.append(this->QTcpSocket::readAll());

    if(duration_time){
        dur_time = 20*b_data.toDouble()+1;
        qDebug() << "VNA duration time from ins: "+QString::number(dur_time)+"\n";
        duration_time = false;
        b_data.clear();
    }else{
        if(!vna_stimulus_values){
            if(b_data.size() > sweep_points*4){
                if(time_for_amplitude){
                    no[0] = b_data.at(1);
                    digits = atoi(no);
                    for(int i=0; i<digits; i++){
                        data.append(b_data.at(2+i));
                    }
                    data=data.left(digits);
                    bytes = data.toUInt();
                    qDebug() << "Digits: "+QString::number(digits)+", Bytes: "+QString::number(bytes);

                    if(bytes == sweep_points*4){
                        b_data.chop(1);
                        b_data=b_data.right(bytes);
                        const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                        for (int i=0; i<b_data.size()/4; ++i){
                            float d = *ptrFloat;
                            if(mag_received){
                                phase.push_back(d);
                            }else{
                                mag.push_back(d);
                            }
                            ptrFloat++;
                        }

                        if(mag_received){
                            temp2d_phase.push_back(phase);
                            nearfield_plot->plot_maxhold();
                            nearfield_plot->add_data_point();
                        }else{
                            temp2d.push_back(mag);
                        }

                        log_file->add_log_line("Current point: ("+QString::number(x_pos)+":"+QString::number(y_pos)+")");
                        log_file->add_log_line("Vector mag of size: "+QString::number(mag.size())+" pushed back to temp2d vector of size: "+QString::number(temp2d.size()));

                        if((x_pos == scan_columns) && mag_received){
                            data_tensor.push_back(temp2d);
                            data_tensor_phase.push_back(temp2d_phase);
                            temp2d.clear();
                            temp2d_phase.clear();
                            log_file->add_log_line("Pushed back to data_tensor vector of size: "+QString::number(data_tensor.size()));
                        }
                        b_data.clear();
                        digits = 0;
                        bytes = 0;

                        if(mag_received){
                            mag.clear();
                            mag_received = false;
                        }else{
                            phase.clear();
                            mag_received = true;
                        }

                    }else{
                        QMessageBox::critical(nullptr, "Scan error!", "Data save error! Scan aborted!");
                        emit stop_scan();
                    }
                }
            }
        }else{
            if(b_data.size() > sweep_points*4){
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
                    qDebug() << "Freq saved. Size: " << freq.size();
                    nearfield_plot->display_first_freq(freq.at(0), freq.at(freq.size()-1));
                    b_data.clear();
                    mag.clear();
                    time_for_amplitude = true;
                    digits = 0;
                    bytes = 0;
                    vna_stimulus_values = false;
                    return;
                }else{
                    emit stop_scan();
                    log_file->add_log_line("Scan error! Frequency data save error! Scan aborted!");
                    QMessageBox::critical(nullptr, "Scan error!", "Freq save error! Scan aborted!");
                }
            }
        }
    }
}

void RS_vna::save_data_to_file(Save sv)
{
    if(sv == FREQ){
        QString path = current_scan_datapath + "xaxis_data.bin";
        std::ofstream file(path.toLocal8Bit(), std::ios::binary);
        if(file.is_open()){
            file.write(reinterpret_cast<const char*>(&freq[0]), freq.size()*sizeof(float));
            file.close();
        }
    }else if(sv == YCOMP){

        //Save magnitudes
        QString path = current_scan_datapath+"y_comp_magn_scan_data_tensor.bin";
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

        //Save phases
        path = current_scan_datapath+"y_comp_phase_scan_data_tensor.bin";

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
        data_tensor_mem_phase = data_tensor_phase;
        if(file.is_open()){
            for(auto &v : data_tensor_phase){
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();

    }else{

        //Save magnitudes
        QString path = current_scan_datapath+"x_comp_magn_scan_data_tensor.bin";
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

        //Save phases
        path = current_scan_datapath+"x_comp_phase_scan_data_tensor.bin";

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
            for(auto &v : data_tensor_phase){
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();
    }
}

void RS_vna::confirm_written_bytes(qint64 bytes)
{
    Q_UNUSED(bytes);
}
