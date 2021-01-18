#include "robot.h"

#include <QThread>
#include <math.h>

Robot::Robot()
{

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

       connect(_socket_robot, SIGNAL(readyRead()), this, SLOT(read_robot_msg()));
       connect(_socket_robot, SIGNAL(bytesWritten(qint64)), this, SLOT(robotBytesWritten(qint64)));

       return true;
    }else{
        return false;
    }
}

void Robot::goto_meas_height(uint16_t x, uint16_t y)
{
    QString msg = "";

    msg = "fast_x = %1\n";
    msg = msg.arg(QString::number(x));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "fast_y = %1\n";
    msg = msg.arg(QString::number(y));
    _socket_robot->write(msg.toLocal8Bit());

    _socket_robot->write("mesheight = 1\n");
    _socket_robot->waitForBytesWritten(10);
}

void Robot::goto_origin(uint16_t x, uint16_t y)
{
    QString msg = "";

    msg = "x_mes = %1\n";
    msg = msg.arg(QString::number(x));
    _socket_robot->write(msg.toLocal8Bit());

    msg = "y_mes = %1\n";
    msg = msg.arg(QString::number(y));
    _socket_robot->write(msg.toLocal8Bit());

    _socket_robot->write("Goto_Origin = 1\n");
    _socket_robot->waitForBytesWritten();
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
    _socket_robot->write("Mes = 1\n");
    _socket_robot->waitForBytesWritten(20);
}

void Robot::stop_scan()
{
    _socket_robot->write("mes_abort = 1\n");
    _socket_robot->waitForBytesWritten(20);
    _socket_robot->write("takepic = 1\n");
    _socket_robot->waitForBytesWritten(20);
}

void Robot::goto_home_pos()
{
    _socket_robot->write("takepic = 1\n");
    _socket_robot->waitForBytesWritten(1000);
}

void Robot::ask_for_camera_height()
{
    _socket_robot->write("cam_h = 1\n");
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

void Robot::set_scan_parameters(uint8_t step_size)
{
    QString msg = "";
    msg = "mes_row_max = %1\n";
    msg = msg.arg(QString::number(scan_area->size_mm.height()/step_size));
    ins->scan_rows = scan_area->size_mm.height()/step_size;
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_column_max = %1\n";
    msg = msg.arg(QString::number(scan_area->size_mm.width()/step_size));
    ins->scan_columns = scan_area->size_mm.width()/step_size;
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_row_res = %1\n";
    msg = msg.arg(QString::number(step_size));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_column_res = %1\n";
    msg = msg.arg(QString::number(step_size));
    _socket_robot->write(msg.toLocal8Bit());
    msg = "mes_delay = 1.5\n";
    _socket_robot->write(msg.toLocal8Bit());

    qDebug() << "X_steps: " << ins->scan_columns;
    qDebug() << "Y_steps: " << ins->scan_rows;
}

void Robot::read_msg()
{
    QByteArray welcome_msg;
    QByteArray msg;
    uint8_t i = 0;

    //Robot welcome message read. If not done, it stays in the buffer.
    if(robot_first_run)
    {
        welcome_msg.append(_socket_robot->readAll());

        if(welcome_msg.at(welcome_msg.size()-1) == '>')
        {
            qDebug() << welcome_msg;
            robot_first_run = false;
        }
    }

    //Take the robot message when available bytes and not thhe welcome message.
    if(_socket_robot->bytesAvailable() && !robot_first_run)
    {
        robot_raw_data.append(_socket_robot->readAll());        //Read all bytes available and append to the QByteArray

        //All robot messages are ended by new line character.
        //Very ofter message is read in more than one package so keep reading
        //and appending until this character reached.
        if(robot_raw_data.at(robot_raw_data.size()-1) == '\n')
        {
            //Extract the information from the robot message. Msg format: @2 123 323 according to the conventions file.
            if(!robot_raw_data.isEmpty())
            {
                qDebug() << "Robot raw message: " << robot_raw_data;
                bool time_for_msg = false;
                for(int t=0; t<robot_raw_data.size(); t++)
                {
                    char a = robot_raw_data.at(t);
                    if(a == '@')
                        time_for_msg = true;
                    if(time_for_msg)
                        msg.append(robot_raw_data.at(t));
                }
                if(!msg.isEmpty())
                {
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
            break;
        case 2:
        {
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
                if(!time_for_col)
                {
                    row_to_cvt.append(a);
                }
                if(time_for_col)
                {
                    col_to_cvt.append(a);
                }
            }

            ins->update_current_position(col_to_cvt.toUInt(), row_to_cvt.toUInt());
            ins->request_magnitudes();

            break;
        }
        case 3:
        {
            if(y_comp)
            {
                emit robot_msg_to_terminal("");
                emit robot_msg_to_terminal("Y comp Scan finished!");
                ins->save_data_to_file('y');
                ins->update_current_position(0, 0);
                ins->reset_saved_data();
                _socket_robot->write("axis_switch=1\n");
            }
            else
            {
                ins->save_data_to_file('x');
                ins->update_current_position(0, 0);

                emit inst_thread_init();
                emit scan_finished_to_wizard();
                emit allow_emit_pos(false);
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
            pcb->send_area_request();
            qDebug() << "Camera distance: " << height;
            break;
        }
        case 17:
        {
            qDebug() << "Probe rotated!";
            y_comp=false;
            _socket_robot->write("Mes=1\n");
        }
        default:
            emit robot_msg_to_terminal("Waiting...");
            break;
    }

}
