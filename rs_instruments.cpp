#include "rs_instruments.h"
#include <qdebug.h>
#include <fstream>
#include <QMessageBox>


RS_Instruments::RS_Instruments(Instrument inst)
{
    ins = inst;
}

RS_Instruments::~RS_Instruments()
{
    delete socket;
}

bool RS_Instruments::tcp_connect(QString)
{
    if(ins==SA)
    {
        socket = new QTcpSocket;
        connect(socket, SIGNAL(readyRead()), this, SLOT(sa_dataread()));
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(confirm_written_bytes(qint64)));

        socket->connectToHost(sa_ip_address, 5025);
        socket->waitForConnected(10);

        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->write("*RST\n");
            socket->write("*ESE 1\n");
            socket->write("SYST:DISP:UPD ON\n");
            socket->write("FORM:DATA REAL,32\n");
            socket->write("INIT:CONT OFF\n");

            return true;
        }
        else
            return false;
    }
    else
    {
        socket = new QTcpSocket;
        connect(socket, SIGNAL(readyRead()), this, SLOT(vna_dataread()));
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(confirm_written_bytes(qint64)));
        socket->connectToHost(vna_ip_address, 5025);
        socket->waitForConnected(10);

        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->write("SYST:TSL OFF\n");
            socket->write("SYST:TSL SCR\n");
            socket->write("SYST:DISP:BAR:STO OFF\n");
            socket->write("SYST:DISP:UPD ON\n");
            socket->write("FORM:DATA REAL,32\n");
            socket->write("INIT:CONT OFF\n");

            return true;
        }
        else
            return false;
    }
}

QTcpSocket *RS_Instruments::give_socket()
{
    return socket;
}

void RS_Instruments::request_freq_values()
{
    if(ins==SA)
    {
        socket->write("TRAC:DATA:X? TRACE1\n");
    }
    else
    {

    }
}

void RS_Instruments::request_magnitudes()
{
    if(!time_for_amplitude)
        time_for_amplitude = true;

    if(ins==SA)
    {
        socket->write("DISP:TRAC1:MODE WRIT\n");
        socket->write("DISP:TRAC1:MODE MAXH\n");
        socket->write("INIT;*WAI\n");
        socket->write("TRAC:DATA? TRACE1;*WAI\n");
    }
    else
    {

    }
}

void RS_Instruments::update_current_position(uint16_t x, uint16_t y)
{
    x_pos = x;
    y_pos = y;
}

void RS_Instruments::get_sweep_points(int sp)
{
    sweep_points = sp;
}

void RS_Instruments::save_data_to_file(char comp)
{
    if(comp == 'y')
    {
        QString path = current_scan_datapath+"y_comp_scan_data_tensor.bin";
        std::ofstream file;

        file.open(path.toLocal8Bit(), std::ios::binary);

        if(file)
        {
            qDebug() << "File does not exists. Created file!";
            float sp = (float)sweep_points;
            float x_max = (float)scan_columns;
            float y_max = (float)scan_rows;

            float step_size_px = 1;
            float scan_width_px = 1;
            float scan_height_px = 1;

            float step_size_mm = (float)ui->stepsize_xy->value();
            float scan_width_mm = (float)scan_area_size.width();
            float scan_height_mm = (float)scan_area_size.height();

            file.write(reinterpret_cast<const char*>(&step_size_px), sizeof(step_size_px));
            file.write(reinterpret_cast<const char*>(&scan_width_px), sizeof(scan_width_px ));
            file.write(reinterpret_cast<const char*>(&scan_height_px), sizeof(scan_height_px));
            file.write(reinterpret_cast<const char*>(&step_size_mm), sizeof(step_size_mm));
            file.write(reinterpret_cast<const char*>(&scan_width_mm), sizeof(scan_width_mm));
            file.write(reinterpret_cast<const char*>(&scan_height_mm), sizeof(scan_height_mm));

            file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
            file.write(reinterpret_cast<const char*>(&x_max), sizeof(x_max));
            file.write(reinterpret_cast<const char*>(&y_max), sizeof(y_max));
        }
        else
        {
            qDebug() << "File exists. No overwrite allowed!";
        }

        qDebug() << "DATA SAVED TO FILE. SIZE OF THE VECTOR FOR Y COMP: " << data_tensor.size();
        if(file.is_open())
        {
            for(auto &v : data_tensor)
            {
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();
    }
    else
    {
        QString path = current_scan_datapath+"x_comp_scan_data_tensor.bin";
        std::ofstream file;

        file.open(path.toLocal8Bit(), std::ios::binary);

        if(file)
        {
            qDebug() << "File does not exists. Created file!";
            float sp = (float)sweep_points;
            float x_max = (float)scan_columns;
            float y_max = (float)scan_rows;

            float step_size_px = 1;
            float scan_width_px = 1;
            float scan_height_px = 1;

            float step_size_mm = (float)ui->stepsize_xy->value();
            float scan_width_mm = (float)scan_area_size.width();
            float scan_height_mm = (float)scan_area_size.height();

            file.write(reinterpret_cast<const char*>(&step_size_px), sizeof(step_size_px));
            file.write(reinterpret_cast<const char*>(&scan_width_px), sizeof(scan_width_px ));
            file.write(reinterpret_cast<const char*>(&scan_height_px), sizeof(scan_height_px));
            file.write(reinterpret_cast<const char*>(&step_size_mm), sizeof(step_size_mm));
            file.write(reinterpret_cast<const char*>(&scan_width_mm), sizeof(scan_width_mm));
            file.write(reinterpret_cast<const char*>(&scan_height_mm), sizeof(scan_height_mm));

            file.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
            file.write(reinterpret_cast<const char*>(&x_max), sizeof(x_max));
            file.write(reinterpret_cast<const char*>(&y_max), sizeof(y_max));
        }
        else
        {
            qDebug() << "File exists. No overwrite allowed!";
        }

        qDebug() << "DATA SAVED TO FILE. SIZE OF THE VECTOR FOR X COMP: " << data_tensor.size();
        if(file.is_open())
        {
            for(auto &v : data_tensor)
            {
                for(auto &v1 : v)
                    file.write(reinterpret_cast<const char*>(&v1[0]), v1.size()*sizeof(float));
            }
        }
        file.close();
    }
}

void RS_Instruments::reset_saved_data()
{
    data_tensor.clear();
    temp2d.clear();
}

void RS_Instruments::sa_dataread()
{
    QByteArray data;
    char no[5];
    uint8_t digits = 0;

    b_data.append(socket->readAll());

    if(b_data.size() > sweep_points*4)
    {
        if(time_for_amplitude)
        {
            no[0] = b_data.at(1);
            digits = atoi(no);
            qDebug() << "Digits: " << digits;
            for(int i=0; i<digits; i++)
            {
                data.append(b_data.at(2+i));
            }
            data=data.left(digits);
            bytes = data.toUInt();
            qDebug() << "Bytes: " << bytes;

            if(bytes == sweep_points*4)
            {
                b_data.chop(1);
                b_data=b_data.right(bytes);
                const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                std::vector<float> mag;
                for (int i=0; i<b_data.size()/4; ++i)
                {
                    float d = *ptrFloat;
                    mag.push_back(d);
                    ptrFloat++;
                }

                temp2d.push_back(mag);
                qDebug() << "Saved magnitude vector of size: " << mag.size() << "To temp2d vector of size: " << temp2d.size();
                qDebug() << "Current point: " << "(" << x_pos << ":" << y_pos << ")";

                if(x_pos == scan_columns)
                {
                    data_tensor.push_back(temp2d);
                    temp2d.push_back(mag);
                    temp2d.clear();
                    qDebug() << "Pushed back to data_tensor vector of size: " << data_tensor.size();
                }
                b_data.clear();
                digits = 0;
                bytes = 0;
            }
            else
            {
                emit stop_scan();
                QMessageBox::critical(nullptr, "Scan error!", "Data save error! Scan aborted!");
            }
        }
        else
        {
            no[0] = b_data.at(1);
            digits = atoi(no);
            qDebug() << "Digits: " << digits;
            for(int i=0; i<digits; i++)
            {
                data[i] = b_data.at(2+i);
            }
            bytes = atoi(data);
            qDebug() << "Bytes: " << bytes;

            if(bytes == sweep_points*4)
            {
                b_data.chop(1);
                b_data=b_data.right(bytes);
                const float* ptrFloat = reinterpret_cast<const float*>(b_data.constData());
                std::vector<float> mag;
                for (int i=0; i<b_data.size()/4; ++i)
                {
                    float d = *ptrFloat;
                    mag.push_back(d);
                    ptrFloat++;
                }

                QString path = current_scan_datapath + "xaxis_data.bin";
                std::ofstream file(path.toLocal8Bit(), std::ios::binary);
                if(file.is_open())
                {
                    file.write(reinterpret_cast<const char*>(&mag[0]), mag.size()*sizeof(float));
                    file.close();
                }
                qDebug() << "Freq saved";
                b_data.clear();
                mag.clear();
                time_for_amplitude = true;
                digits = 0;
                bytes = 0;
                return;
            }
            else
            {
                emit stop_scan();
                QMessageBox::critical(nullptr, "Scan error!", "Data save error! Scan aborted!");
            }
        }
    }
}

void RS_Instruments::confirm_written_bytes(qint64 bytes)
{
    Q_UNUSED(bytes);
}


