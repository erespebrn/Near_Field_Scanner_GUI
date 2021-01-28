#include "rs_instruments.h"
#include <qdebug.h>
#include <fstream>
#include <QMessageBox>


RS_Instruments::RS_Instruments(Instrument inst, QString ip)
{
    ins = inst;
    ip_address = ip;
    log_file = new Event_log;
}

RS_Instruments::~RS_Instruments()
{
    delete socket;
    delete log_file;
}

bool RS_Instruments::tcp_connect()
{
    if(ins==SA){
        socket = new QTcpSocket;
        connect(socket, SIGNAL(readyRead()), this, SLOT(sa_dataread()));
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(confirm_written_bytes(qint64)));

        socket->connectToHost(ip_address, 5025);
        socket->waitForConnected(10);

        if(socket->state() == QAbstractSocket::ConnectedState){
            socket->write("*RST\n");
            socket->write("*ESE 1\n");
            socket->write("SYST:DISP:UPD ON\n");
            socket->write("FORM:DATA REAL,32\n");
            socket->write("INIT:CONT OFF\n");
            return true;
        }else
            return false;
    }else{
        socket = new QTcpSocket;
        connect(socket, SIGNAL(readyRead()), this, SLOT(vna_dataread()));
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(confirm_written_bytes(qint64)));
        socket->connectToHost(ip_address, 5025);
        socket->waitForConnected(10);

        if(socket->state() == QAbstractSocket::ConnectedState){
            //socket->write("SYST:TSL OFF\n");
            //socket->write("SYST:TSL SCR\n");
            socket->write("*RST\n");
            socket->write("SYST:DISP:BAR:STO OFF\n");
            socket->write("SYST:DISP:UPD ON\n");
            socket->write("FORM:DATA REAL,32\n");
            socket->write("INIT:CONT OFF\n");
            return true;
        }else
            return false;
    }
}

void RS_Instruments::init_scan()
{
    if(!QDir(current_scan_datapath).exists())
        QDir().mkdir(current_scan_datapath);
}

void RS_Instruments::request_freq_values()
{
    if(ins==SA){
        log_file->add_log_line("SA freq request");
        socket->write("TRAC:DATA:X? TRACE1\n");
    }else{
        log_file->add_log_line("VNA freq request");
        vna_stimulus_values = true;
        socket->write("TRAC:DATA:STIM? CH1DATA;*WAI\n");
    }
}

void RS_Instruments::request_magnitudes()
{
    if(!time_for_amplitude)
        time_for_amplitude = true;

    if(ins==SA){
        log_file->add_log_line("SA data request");
        socket->write("DISP:TRAC1:MODE WRIT\n");
        socket->write("DISP:TRAC1:MODE MAXH\n");
        socket->write("INIT;*WAI\n");
        socket->write("TRAC:DATA? TRACE1;*WAI\n");
    }else{
        log_file->add_log_line("VNA data request");
        socket->write("INIT1;*WAI\n");
        socket->write("TRAC? CH1DATA;*WAI\n");
    }
}

void RS_Instruments::request_duration_time()
{
    if(ins==SA){
        socket->write("SWE:DUR?\n");
    }else{
        socket->write("SWE:TIME?\n");
    }
    duration_time = true;
}

void RS_Instruments::get_datapath(QString dp)
{
    current_scan_datapath = dp;
}

void RS_Instruments::sa_dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;

    b_data.append(socket->readAll());

    if(duration_time){
        dur_time = 2*(no_of_sweeps*b_data.toDouble())+1;
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
                    std::vector<float> mag;
                    for (int i=0; i<b_data.size()/4; ++i){
                        float d = *ptrFloat;
                        mag.push_back(d);
                        ptrFloat++;
                    }
                    temp2d.push_back(mag);

                    log_file->add_log_line("Current point: ("+QString::number(x_pos)+":"+QString::number(y_pos)+")");
                    log_file->add_log_line("Vector mag of size: "+QString::number(mag.size())+" pushed back to temp2d vector of size: "+QString::number(temp2d.size()));

                    if(x_pos == scan_columns){
                        data_tensor.push_back(temp2d);
                        temp2d.push_back(mag);
                        temp2d.clear();
                        log_file->add_log_line("Pushed back to data_tensor vector of size: "+QString::number(data_tensor.size()));
                    }
                    b_data.clear();
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
                    std::vector<float> mag;
                    for (int i=0; i<b_data.size()/4; ++i){
                        float d = *ptrFloat;
                        mag.push_back(d);
                        ptrFloat++;
                    }

                    QString path = current_scan_datapath + "xaxis_data.bin";
                    std::ofstream file(path.toLocal8Bit(), std::ios::binary);
                    if(file.is_open()){
                        file.write(reinterpret_cast<const char*>(&mag[0]), mag.size()*sizeof(float));
                        file.close();
                    }
                    log_file->add_log_line("Freq saved");
                    b_data.clear();
                    mag.clear();
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

void RS_Instruments::vna_dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;

    b_data.append(socket->readAll());

    if(duration_time){
        dur_time = b_data.toDouble()+1;
        if(scan_started)
            log_file->add_log_line("VNA duration time from ins: "+QString::number(dur_time)+"\n");
        duration_time = false;
        b_data.clear();
    }else{
        if(!vna_stimulus_values)
        {
            if(b_data.size() > sweep_points*8){
                if(time_for_amplitude){
                    no[0] = b_data.at(1);
                    digits = atoi(no);
                    for(int i=0; i<digits; i++){
                        data.append(b_data.at(2+i));
                    }
                    data=data.left(digits);
                    bytes = data.toUInt();
                    log_file->add_log_line("Digits: "+QString::number(digits)+", Bytes: "+QString::number(bytes));

                    if(bytes == sweep_points*8){
                        b_data.chop(1);
                        b_data=b_data.right(bytes);
                        const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                        std::vector<float> mag;
                        for (int i=0; i<b_data.size()/4; ++i){
                            float d = *ptrFloat;
                            mag.push_back(d);
                            ptrFloat++;
                        }

                        temp2d.push_back(mag);
                        log_file->add_log_line("Current point: ("+QString::number(x_pos)+":"+QString::number(y_pos)+")");
                        log_file->add_log_line("Vector mag of size: "+QString::number(mag.size())+" pushed back to temp2d vector of size: "+QString::number(temp2d.size()));

                        if(x_pos == scan_columns){
                            data_tensor.push_back(temp2d);
                            temp2d.push_back(mag);
                            temp2d.clear();
                            log_file->add_log_line("Pushed back to data_tensor vector of size: "+QString::number(data_tensor.size()));
                        }
                        b_data.clear();
                        digits = 0;
                        bytes = 0;
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
                    std::vector<float> mag;
                    for (int i=0; i<b_data.size()/4; ++i){
                        float d = *ptrFloat;
                        mag.push_back(d);
                        ptrFloat++;
                    }

                    QString path = current_scan_datapath + "xaxis_data.bin";
                    std::ofstream file(path.toLocal8Bit(), std::ios::binary);
                    if(file.is_open()){
                        file.write(reinterpret_cast<const char*>(&mag[0]), mag.size()*sizeof(float));
                        file.close();
                    }
                    log_file->add_log_line("Freq saved");
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

void RS_Instruments::save_data_to_file(char comp)
{
    if(comp == 'y'){
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
        }else{
            log_file->add_log_line("File exists. No overwrite allowed!");
        }

        log_file->add_log_line("MAGNITUDES DATA SAVED TO FILE. SIZE OF THE VECTOR FOR Y COMP: "+QString::number(data_tensor.size()));
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

        log_file->add_log_line("FREQUENCIES DATA SAVED TO FILE. SIZE OF THE VECTOR FOR Y COMP: "+QString::number(data_tensor.size()));
        if(file.is_open()){
            for(auto &v : data_tensor){
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();
    }
}

void RS_Instruments::update_current_position(uint16_t x, uint16_t y)
{
    x_pos = x;
    y_pos = y;
}

void RS_Instruments::reset_saved_data()
{
    data_tensor.clear();
    temp2d.clear();
}

QTcpSocket *RS_Instruments::give_socket()
{
    return socket;
}

void RS_Instruments::confirm_written_bytes(qint64 bytes)
{
    Q_UNUSED(bytes);
}


