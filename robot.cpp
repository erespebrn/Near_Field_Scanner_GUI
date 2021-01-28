#include "robot.h"

#include <QThread>
#include <math.h>

Robot::Robot()
{

}

Robot::~Robot()
{
    delete _socket_robot;
}

bool Robot::tcp_connect()
{
    // *** Robot TCP connection *** //
    _socket_robot = new QTcpSocket;
    _socket_robot->connectToHost(robot_ip_address, 23);
    _socket_robot->waitForConnected();

    if(_socket_robot->state() == QAbstractSocket::ConnectedState){
       _socket_robot->write("");
       _socket_robot->waitForBytesWritten(40);

       _socket_robot->write("as\n");
       _socket_robot->waitForBytesWritten(20);

       //QThread::sleep(1);

       _socket_robot->write("ZPOWER ON\n");
       _socket_robot->waitForBytesWritten(20);

       _socket_robot->write("EXECUTE main\n");
       // *** //

       connect(_socket_robot, SIGNAL(readyRead()), this, SLOT(read_msg()));
       connect(_socket_robot, SIGNAL(bytesWritten(qint64)), this, SLOT(robotBytesWritten(qint64)));

       return true;
    }else{
        return false;
    }
}

void Robot::read_msg()
{
    QByteArray welcome_msg;
    QByteArray msg;
    uint8_t i = 0;
    //Robot welcome message read. If not done, it stays in the buffer.
    if(robot_first_run){
        welcome_msg.append(_socket_robot->readAll());
        if(welcome_msg.at(welcome_msg.size()-1) == '>'){
            qDebug() << welcome_msg;
            robot_first_run = false;
        }
    }

    if(start_scan_error){
        if(cnt++ > 1){
            this->stop_scan();
            emit scan_error();
            start_scan_error = false;
            cnt = 0;
            return;
        }
    }

    //Take the robot message when available bytes and not thhe welcome message.
    if(_socket_robot->bytesAvailable() && !robot_first_run){
        robot_raw_data.append(_socket_robot->readAll());        //Read all bytes available and append to the QByteArray

        //All robot messages are ended by new line character.
        //Very ofter message is read in more than one package so keep reading
        //and appending until this character reached.
        if(robot_raw_data.at(robot_raw_data.size()-1) == '\n'){
            //Extract the information from the robot message. Msg format: @2 123 323 according to the conventions file.
            if(!robot_raw_data.isEmpty()){
                qDebug() << "Robot raw message: " << robot_raw_data;
                bool time_for_msg = false;
                for(int t=0; t<robot_raw_data.size(); t++){
                    char a = robot_raw_data.at(t);
                    if(a == '@')
                        time_for_msg = true;
                    if(time_for_msg)
                        msg.append(robot_raw_data.at(t));
                }
                if(!msg.isEmpty()){
                    char arg_to_cvt[5];
                    arg_to_cvt[0] = msg.at(1);
                    arg_to_cvt[1] = msg.at(2);
                    arg_to_cvt[2] = '\n';
                    i = atoi(arg_to_cvt);
                }
                robot_raw_data.clear();
            }
        }
    }

    //Put the extracted information for the correct handling.
    switch(i)
    {
        case 1:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Scan started");
            start_scan_error = true;
            break;
        case 2:
        {
            if(start_scan_error)
                start_scan_error = false;


            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Scan in progress...");

            //Extract the current scan row and column from the robot message.
            QByteArray col_to_cvt;
            QByteArray row_to_cvt;
            bool time_for_col = false;

            for(size_t p=3; p<strlen(msg); p++)
            {
                char a = msg.at(p);
                if(a == ' ')
                    time_for_col = true;
                if(!time_for_col){
                    row_to_cvt.append(a);
                }
                if(time_for_col){
                    col_to_cvt.append(a);
                }
            }

            ins->update_current_position(col_to_cvt.toUInt(), row_to_cvt.toUInt());
            ins->request_magnitudes();

            break;
        }
        case 3:
        {
            if(x_comp_sel && y_comp_sel){
                if(y_comp){
                    ins->save_data_to_file('y');
                    ins->update_current_position(1, 1);
                    ins->reset_saved_data();
                    probe_rotation = true;
                    _socket_robot->write("axis_switch=1\n");
                    scan_started=false;
                    emit robot_msg_to_terminal("");
                    emit robot_msg_to_terminal("Y comp Scan finished!");
                }
                if(!probe_rotation){
                    ins->save_data_to_file('x');
                    ins->update_current_position(1, 1);
                    ins->reset_saved_data();
                    emit inst_thread_init();
                    emit scan_finished_to_wizard();
                    emit allow_emit_pos(false);
                    emit save_cropped_image();
                }
                else{
                    _socket_robot->write("axis_switch=1\n");
                }
            }else if(y_comp_sel && !x_comp_sel){
                ins->save_data_to_file('y');
                ins->update_current_position(1, 1);
                ins->reset_saved_data();
                scan_started=false;
                emit robot_msg_to_terminal("");
                emit robot_msg_to_terminal("Scan finished!");
                emit inst_thread_init();
                emit scan_finished_to_wizard();
                emit allow_emit_pos(false);
                emit save_cropped_image();
            }else if(!y_comp_sel && x_comp_sel){
                ins->save_data_to_file('x');
                ins->update_current_position(1, 1);
                ins->reset_saved_data();
                scan_started=false;
                emit robot_msg_to_terminal("");
                emit robot_msg_to_terminal("Scan finished!");
                emit inst_thread_init();
                emit scan_finished_to_wizard();
                emit allow_emit_pos(false);
                emit save_cropped_image();
            }
            break;
        }
        case 4:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Height measure started...");
            emit stop_displaying_point();
            break;
        case 5:
        {
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Height measure done!");

            char value_to_cvt[10];

            for(size_t i=3; i<strlen(msg); i++)
                value_to_cvt[i-3] = msg.at(i);
            float height = 0.0;
            height = atof(value_to_cvt);
            height = roundf(height);
            DUT_size::real_height = int(sqrt(pow(height,2)));
            emit height_measured();
            break;
        }
        case 6:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Robot position");
            break;
        case 7:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Going to the PCB's corner...");
            break;
        case 8:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Reached the PCB's corner!");
            break;
        case 9:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Moving to the homeposition...");
            break;
        case 10:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Reached the homeposition!");
            break;
        case 11:
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Scan aborted!");
            emit inst_thread_init();
            ins->update_current_position(0, 0);
            emit allow_emit_pos(false);
            break;
        case 15:
        {
            char value_to_cvt[10];
            for(size_t i=3; i<strlen(msg); i++)
                value_to_cvt[i-3] = msg.at(i);
            float height = 0.0;
            height = atof(value_to_cvt);
            height = roundf(height);

            DUT_size::camera_distance_2 = height;

            if(emit_area)
                pcb->send_area_request();

            qDebug() << "Camera distance: " << height;
            break;
        }
        case 17:
        {
            if(probe_rotation){
                probe_rotation = false;
                _socket_robot->write("Mes = 1\n");
                y_comp=false;
            }
            if(scan_started)
                ins->log_file->add_log_line("Probe rotated");
            emit robot_msg_to_terminal("Probe rotated!");
            break;
        }
        default:
            emit robot_msg_to_terminal("Waiting...");
            break;
    }

}

void Robot::X_shift(double x)
{
    QString msg = "xShift = %1\n";
    msg = msg.arg(QString::number(x));
    _socket_robot->write(msg.toLocal8Bit());
}

void Robot::Y_shift(double y)
{
    QString msg = "yShift = %1\n";
    msg = msg.arg(QString::number(y));
    _socket_robot->write(msg.toLocal8Bit());
}

void Robot::probe_rotate()
{
    _socket_robot->write("axis_switch=1\n");
}

void Robot::Z_shift(double z)
{
    QString msg = "zShift = %1\n";
    msg = msg.arg(QString::number(z));
    _socket_robot->write(msg.toLocal8Bit());
}

void Robot::scan_heigh_update(double h)
{
    QString msg = "measuring_height = %1\n";
    msg = msg.arg(QString::number(h));
    _socket_robot->write(msg.toLocal8Bit());
}

void Robot::tool_change(int t_x, int t_y, int t_z)
{
    ///Set tl_x,tl_y,tl_z to the appropriate values
    QString msg = "tl_x = %1\n";
    msg = msg.arg(QString::number(t_x));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "tl_y = %1\n";
    msg = msg.arg(QString::number(t_y));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "tl_z = %1\n";
    msg = msg.arg(QString::number(t_z));
    _socket_robot->write(msg.toLocal8Bit());

    ///Call the robot's toolchange function
    _socket_robot->write("tool_change = 1\n");
}

void Robot::goto_meas_height(uint16_t x, uint16_t y)
{
    QString msg = "";

    msg = "fast_x = %1\n";
    msg = msg.arg(QString::number(x+height_off_x));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "fast_y = %1\n";
    msg = msg.arg(QString::number(y+height_off_y));
    _socket_robot->write(msg.toLocal8Bit());

    _socket_robot->write("mesheight = 1\n");
}

void Robot::goto_origin(uint16_t x, uint16_t y)
{
    QString msg = "";

    msg = "x_mes = %1\n";
    msg = msg.arg(QString::number(x+start_scan_off_x));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "y_mes = %1\n";
    msg = msg.arg(QString::number(y+start_scan_off_y));
    _socket_robot->write(msg.toLocal8Bit());

    _socket_robot->write("Goto_Origin = 1\n");
}

void Robot::goto_takepic2_pos(uint16_t x, uint16_t y)
{
    QString msg = "";

    int h = sqrt(pow(x,2)+pow(y,2));

    msg = "zShift = %1\n";
    msg = msg.arg(QString::number(h));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "yShift = %1\n";
    msg = msg.arg(QString::number(-30));
    _socket_robot->write(msg.toLocal8Bit());
}

void Robot::start_scan()
{
    ins->init_scan();
    ins->request_freq_values();
    scan_started = true;
    if(!y_comp_sel && x_comp_sel){
        probe_rotate();
    }
    for(int i=0; i<3000; i++){};
    _socket_robot->write("Mes = 1\n");
}

void Robot::stop_scan()
{
    scan_started = false;
    _socket_robot->write("mes_abort = 1\n");
    _socket_robot->write("takepic = 1\n");
}

void Robot::goto_home_pos()
{
    _socket_robot->write("takepic = 1\n");
}

void Robot::ask_for_camera_height()
{
    _socket_robot->write("cam_h = 1\n");
}

void Robot::allow_emit_area(bool b)
{
    emit_area = b;
}

void Robot::assign_rs_instrument(RS_Instruments *i)
{
    ins = i;
}

void Robot::assign_duts(DUT_size *p, DUT_size *s)
{
    pcb = p;
    scan_area = s;
}

void Robot::set_scan_parameters(double step_size)
{
    ins->scan_columns = round(scan_area->size_mm.width()/step_size);
    ins->scan_rows = round(scan_area->size_mm.height()/step_size);
    total_time = round(ins->dur_time*(ins->scan_columns)*(ins->scan_rows));

    if(x_comp_sel && y_comp_sel)
        total_time*=2;

    ins->log_file->add_log_line("X_steps: "+QString::number(ins->scan_columns));
    ins->log_file->add_log_line("Y_steps: "+QString::number(ins->scan_rows));
    ins->log_file->add_log_line("Estimated scan duration: "+QString::number(total_time));

    QString msg = "";
    msg = "mes_row_max = %1\n";
    msg = msg.arg(QString::number(ins->scan_rows));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_column_max = %1\n";
    msg = msg.arg(QString::number(ins->scan_columns));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_row_res = %1\n";
    msg = msg.arg(QString::number(step_size));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_column_res = %1\n";
    msg = msg.arg(QString::number(step_size));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_delay = %1\n";
    msg = msg.arg(QString::number(ins->dur_time));
    _socket_robot->write(msg.toLocal8Bit());
}

void Robot::robotBytesWritten(qint64 b)
{
    Q_UNUSED(b);
}
