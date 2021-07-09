#include "robot.h"

#include <QThread>
#include <math.h>

Robot::Robot() : Tcp_device()
{}

Robot::~Robot()
{}

bool Robot::init()
{
    // *** Robot TCP connection *** //

    if(this->Tcp_device::tcp_connect(robot_ip_address, robot_port)){
       this->QTcpSocket::write("");
       this->QTcpSocket::waitForBytesWritten(40);

       this->QTcpSocket::write("as\n");
       this->QTcpSocket::waitForBytesWritten(20);

       QThread::sleep(1);

       this->QTcpSocket::write("ZPOWER ON\n");
       this->QTcpSocket::waitForBytesWritten(20);

       this->QTcpSocket::write("EXECUTE main\n");
       // *** //

       connect(this, &QTcpSocket::readyRead, this, &Robot::read_msg);
       connect(this, &QTcpSocket::bytesWritten, this, &Robot::robotBytesWritten);

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
    //Robot welcome message read. If not done, it stays in the buffer and mess up.
    if(robot_first_run){
        welcome_msg.append(this->QTcpSocket::readAll());
        if(welcome_msg.at(welcome_msg.size()-1) == '>'){
            //qDebug() << welcome_msg;
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
    if(this->QTcpSocket::bytesAvailable() && !robot_first_run){
        robot_raw_data.append(this->QTcpSocket::readAll());        //Read all bytes available and append to the QByteArray

        //All robot messages are ended by new line character.
        //Very often message is read in more than one package so keep reading
        //and appending until this character reached.
        if(robot_raw_data.at(robot_raw_data.size()-1) == '\n'){

            //Extract the information from the robot message. Msg format: @2 123 323 according to the conventions file.
            if(!robot_raw_data.isEmpty()){
                //qDebug() << "Robot raw message: " << robot_raw_data;
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
        case 1: //Scan started case
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Scan started");
            start_scan_error = true;
            break;
        case 2: //Scan in progress case. Gives also the current robot position
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

            //Update the current position and request magnitudes from the RS Instrument
            ins->update_current_position(col_to_cvt.toUInt(), row_to_cvt.toUInt());
            ins->request_magnitudes();

            break;
        }
        case 3: //Scan finished (or component finished) case
        {
            //Handle the end of signgle component/both components scan end
            if(x_comp_sel && y_comp_sel){
                if(y_comp){

                    //Instruments methods
                    ins->save_data_to_file(RS_Instruments::YCOMP);
                    ins->reset_saved_data();
                    ins->update_current_position(1, 1);

                    //Rotate the axis robot command
                    this->probe_rotate();

                    //Some flags
                    scan_started   = false;
                    probe_rotation = true;

                    //Emit the info to the main window
                    emit robot_msg_to_terminal("");
                    emit robot_msg_to_terminal("Y comp Scan finished!");
                }
                if(!probe_rotation){

                    //Instruments methods
                    ins->save_data_to_file(RS_Instruments::XCOMP);
                    ins->update_current_position(1, 1);

                    //Some flags
                    scan_started     = false;
                    start_scan_error = false;
                    x_comp_sel       = false;
                    y_comp_sel       = false;
                    y_comp           = true;

                    //Couter for error detection
                    cnt = 0;

                    //End the scan
                    this->stop_scan();
                    this->goto_home_pos();

                    //Emit the info to the main window
                    emit this->robot_msg_to_terminal("");
                    emit this->robot_msg_to_terminal("Scan finished!");

                    //Emit scan finished signal to scan interface
                    emit this->scan_finished();

                }else{
                    //Rotate the axis robot command
                    this->probe_rotate();
                }
            }else if(y_comp_sel && !x_comp_sel){

                //Instruments methods
                ins->save_data_to_file(RS_Instruments::YCOMP);
                ins->update_current_position(1, 1);
                ins->allow_nearfield_plot_exports(true);

                //Some flags
                scan_started     = false;
                start_scan_error = false;
                x_comp_sel       = false;
                y_comp_sel       = false;
                y_comp           = true;

                //Counter for error detection
                cnt = 0;

                //End the scan
                this->stop_scan();
                this->goto_home_pos();

                //Emit the info to the main window
                emit robot_msg_to_terminal("");
                emit robot_msg_to_terminal("Scan finished!");

                //Emit scan finished signal to scan interface
                emit scan_finished();

            }else if(!y_comp_sel && x_comp_sel){

                //Instruments methods
                ins->save_data_to_file(RS_Instruments::XCOMP);
                ins->update_current_position(1, 1);
                ins->allow_nearfield_plot_exports(true);

                //Some flags
                scan_started     = false;
                start_scan_error = false;
                x_comp_sel       = false;
                y_comp_sel       = false;
                y_comp           = true;

                //Counter for error detection
                cnt = 0;

                //End the scan
                this->stop_scan();
                this->goto_home_pos();

                //Emit the info to the main window
                emit robot_msg_to_terminal("");
                emit robot_msg_to_terminal("Scan finished!");

                //Emit scan finished signal to scan interface
                emit scan_finished();
            }
            break;
        }
        case 4: //Height measurement started case
        {
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Height measurement started...");

            //Stop displaying the point for height measurement
            emit stop_displaying_point();

            break;
        }
        case 5: //Heigth measurement done case
        {
            //Extract the height from the robot msg
            char value_to_cvt[10];
            for(size_t i=3; i<strlen(msg); i++)
                value_to_cvt[i-3] = msg.at(i);
            float height = 0.0;
            height = atof(value_to_cvt);
            height = roundf(height);
            DUT_size::real_height = int(sqrt(pow(height,2)));

            //Emit that the height has been measured
            emit height_measured();

            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Height measure done!");
            break;
        }
        case 6:
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Robot position");
            break;
        case 7:
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Going to the PCB's corner...");
            break;
        case 8:
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Reached the PCB's corner!");
            break;
        case 9:
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Moving to the homeposition...");
            break;
        case 10:
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Reached the homeposition!");
            break;
        case 11: // Scan aborted case
        {
            //Instruments methods
            ins->update_current_position(1, 1);

            //Some flags
            start_scan_error = false;
            x_comp_sel = false;
            y_comp_sel = false;
            y_comp = true;

            //Counter for error detection
            cnt = 0;

            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Scan aborted!");
            break;
        }
        case 15: //Camera distance received case
        {
            //Extract the camera distance
            char value_to_cvt[10];
            for(size_t i=3; i<strlen(msg); i++)
                value_to_cvt[i-3] = msg.at(i);
            float height = 0.0;
            height = atof(value_to_cvt);
            //height = roundf(height);
            DUT_size::camera_distance_2 = height;
            break;
        }
        case 17:
        {
            //If time to rotate probe for scanning the x component
            if(probe_rotation){

                //Set the current component for nearfield plot
                ins->set_nearfield_plot_current_scan_comp(NF_plot::XCOMP);

                //Reset some flags
                probe_rotation = false;
                y_comp         = false;

                //Add line to the log file
                log_file->add_log_line("Probe rotated");

                //Start the x-comp scan after the probe rotation
                this->start_scan();
            }

            //Emit the info to the main window
            emit robot_msg_to_terminal("Probe rotated!");
            break;
        }
        case 21:
        {
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Y component calibration started");
            break;
        }
        case 22:
        {
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("Y component calibration finished");
            break;
        }
        case 23:
        {
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("X component calibration started");
            break;
        }
        case 24:
        {
            //Emit the info to the main window
            emit robot_msg_to_terminal("");
            emit robot_msg_to_terminal("X component calibration finished");
            break;
        }
        case 25:
        {
            this->ask_for_camera_height();
            break;
        }
        default:
            //Emit the info to the main window
            emit robot_msg_to_terminal("Idle...");
            break;
    }

}

void Robot::probe_rotate()
{
    QString msg = "X_axis = %1\n";
    msg = msg.arg(QString::number(x_axis));
    this->QTcpSocket::write(msg.toLocal8Bit());
    x_axis = !x_axis;
    qDebug() << "X_axis: " << x_axis;
}

void Robot::calib_x_enable(bool en)
{
    en ? this->QTcpSocket::write("calibX = 1\n") : this->QTcpSocket::write("calibX = 0\n");
}

void Robot::calib_y_enable(bool en)
{
    en ? this->QTcpSocket::write("calibY = 1\n") : this->QTcpSocket::write("calibY = 0\n");
    switch_arrows = en;
}

void Robot::X_shift(double x)
{
    QString msg = "";

    if(switch_arrows){
        msg = "yShift = %1\n";
        msg = msg.arg(QString::number(-x));
    }else{
        msg = "xShift = %1\n";
        msg = msg.arg(QString::number(x));
    }
    this->QTcpSocket::write(msg.toLocal8Bit());
}

void Robot::Y_shift(double y)
{
    QString msg = "";
    switch_arrows ? msg = "xShift = %1\n" : msg = "yShift = %1\n";
    msg = msg.arg(QString::number(y));
    this->QTcpSocket::write(msg.toLocal8Bit());
}

void Robot::Z_shift(double z)
{
    QString msg = "zShift = %1\n";
    msg = msg.arg(QString::number(z));
    this->QTcpSocket::write(msg.toLocal8Bit());
}

void Robot::scan_heigh_update(double h)
{
    QString msg = "measuring_height = %1\n";
    msg = msg.arg(QString::number(h));
    this->QTcpSocket::write(msg.toLocal8Bit());

    first_scan_height_update ? first_scan_height_update = false : this->QTcpSocket::write("Goto_Origin = 1\n");
}

void Robot::goto_meas_height(float x, float y)
{
    QString msg = "";

    qDebug() << "fast_x from GUI: " << x << "\nfast_y from GUI: " << y;

    msg = "fast_x = %1\n";
    msg = msg.arg(QString::number(x));
    this->QTcpSocket::write(msg.toLocal8Bit());

    msg = "fast_y = %1\n";
    msg = msg.arg(QString::number(y));
    this->QTcpSocket::write(msg.toLocal8Bit());

    this->QTcpSocket::write("mesheight = 1\n");
}

void Robot::goto_origin(float x, float y)
{
    QString msg = "";

    qDebug() << "X mes from GUI: " << x << "\nY mes from GUI: " << y;

    msg = "x_mes = %1\n";
    msg = msg.arg(QString::number(x));
    this->QTcpSocket::write(msg.toLocal8Bit());

    msg = "y_mes = %1\n";
    msg = msg.arg(QString::number(y));
    this->QTcpSocket::write(msg.toLocal8Bit());

    this->QTcpSocket::write("Goto_Origin = 1\n");
}

void Robot::goto_takepic2_pos(float x, float y)
{
    QString msg = "";

    int h = sqrt(pow(x,2)+pow(y,2));

    msg = "zShift = %1\n";
    msg = msg.arg(QString::number(h));
    this->QTcpSocket::write(msg.toLocal8Bit());

    msg = "yShift = %1\n";
    msg = msg.arg(QString::number(-30));
    this->QTcpSocket::write(msg.toLocal8Bit());
}

void Robot::set_takepic2_calib()
{
    this->QTcpSocket::write("SetPos = 1\n");
}

void Robot::goto_takepic2_calib()
{
    this->QTcpSocket::write("secondTakepic = 1\n");
}

void Robot::start_scan()
{
    scan_started = true;
    if(!y_comp_sel && x_comp_sel){
        ins->set_nearfield_plot_current_scan_comp(NF_plot::XCOMP);
        this->probe_rotate();
    }
    first_scan_height_update = true;
    for(int i=0; i<3000; i++){};
    this->QTcpSocket::write("Mes = 1\n");
}

void Robot::stop_scan()
{
    scan_started = false;
    this->QTcpSocket::write("mes_abort = 1\n");
    this->QTcpSocket::write("takepic = 1\n");
}

void Robot::goto_home_pos()
{
    this->QTcpSocket::write("takepic = 1\n");
    first_scan_height_update = true;
    x_axis = true;
}

void Robot::ask_for_camera_height()
{
    this->QTcpSocket::write("cam_h = 1\n");
}

void Robot::allow_emit_area(bool b)
{
    emit_area = b;
}

void Robot::set_scan_parameters(double step_size)
{
    ins->set_scan_columns(round(scan_area->size_mm.width()/step_size)+1);
    ins->set_scan_rows(round(scan_area->size_mm.height()/step_size)+1);

    total_time = 1.5+round((ins->get_duration_time()+0.68)*(ins->get_scan_columns())*(ins->get_scan_rows()));

    if(x_comp_sel && y_comp_sel)
        total_time*=2;

    log_file->add_log_line("X_steps: "+QString::number(ins->get_scan_columns()));
    log_file->add_log_line("Y_steps: "+QString::number(ins->get_scan_rows()));
    log_file->add_log_line("Estimated scan duration: "+QString::number(total_time));

    QString msg = "";
    msg = "mes_row_max = %1\n";
    msg = msg.arg(QString::number(ins->get_scan_rows()));
    this->QTcpSocket::write(msg.toLocal8Bit());
    msg = "mes_column_max = %1\n";
    msg = msg.arg(QString::number(ins->get_scan_columns()));
    this->QTcpSocket::write(msg.toLocal8Bit());
    msg = "mes_row_res = %1\n";
    msg = msg.arg(QString::number(step_size));
    this->QTcpSocket::write(msg.toLocal8Bit());
    msg = "mes_column_res = %1\n";
    msg = msg.arg(QString::number(step_size));
    this->QTcpSocket::write(msg.toLocal8Bit());
    msg = "mes_delay = %1\n";
    msg = msg.arg(QString::number(ins->get_duration_time()));
    this->QTcpSocket::write(msg.toLocal8Bit());
}

void Robot::tool_change(int t_x, int t_y, int t_z)
{
    ///Set tl_x,tl_y,tl_z to the appropriate values
    QString msg = "tl_x = %1\n";
    msg = msg.arg(QString::number(t_x));
    this->QTcpSocket::write(msg.toLocal8Bit());

    msg = "tl_y = %1\n";
    msg = msg.arg(QString::number(t_y));
    this->QTcpSocket::write(msg.toLocal8Bit());

    msg = "tl_z = %1\n";
    msg = msg.arg(QString::number(t_z));
    this->QTcpSocket::write(msg.toLocal8Bit());

    ///Call the robot's toolchange function
    this->QTcpSocket::write("tool_change = 1\n");
}
