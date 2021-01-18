#ifndef ROBOT_H
#define ROBOT_H

#include <QWidget>
#include <QObject>
#include <QTimer>
#include <QString>
#include <QTcpSocket>

#include "rs_instruments.h"
#include "dut_size.h"

class Robot : public QObject {

    Q_OBJECT
public:
    Robot();
    /**
     * @brief Connects to the robot via TCP socket.
     */
    bool tcp_connect();
    /**
     * @brief Measure height at the given position 'x' and 'y'.
     */
    void goto_meas_height(uint16_t x, uint16_t y);
    /**
     * @brief Moves to the given position x, y in 2D plane.
     */
    void goto_origin(uint16_t x, uint16_t y);
    /**
     * @brief Moves to the takepic2 position. Use only after the height measured. Give pcb_width and pcb_height.
     */
    void goto_takepic2_pos(uint16_t pcb_width, uint16_t pcb_height);
    /**
     * @brief Start the scan.
     */
    void start_scan();

    void stop_scan();
    void goto_home_pos();
    void ask_for_camera_height();
    void assign_rs_instrument(RS_Instruments*);
    void assign_duts(DUT_size*, DUT_size*);
    void set_scan_parameters(uint8_t);

    QTcpSocket *_socket_robot;
private slots:
     void read_msg();

signals:
    void robot_msg_to_terminal(QString);
    void scan_finished_to_wizard();
    void allow_emit_pos(bool);
    void stop_displaying_point();
    void height_measured();
    void inst_thread_init();
    void send_area_to_videothread();
    void send_real_pcb_height(uint32_t);

private:
    RS_Instruments *ins;
    DUT_size *pcb;
    DUT_size *scan_area;
    QString robot_ip_address = "192.168.11.2";
    QByteArray robot_raw_data;
    bool robot_first_run = true;
    bool y_comp = true;
};

#endif // ROBOT_H
