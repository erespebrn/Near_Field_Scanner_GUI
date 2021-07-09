#ifndef ROBOT_H
#define ROBOT_H

#include <QWidget>
#include <QObject>
#include <QTimer>
#include <QString>
#include <QTcpSocket>
#include <memory>

#include "Scan_process/PCB_size/dut_size.h"
#include "TCP_IP/tcp_device.h"
#include "TCP_IP/RS_Instr/rs_instruments.h"

class Robot : public Tcp_device
{

    Q_OBJECT
public:
    Robot(void);
    ~Robot(void);

    bool        init(void);
    void        goto_meas_height(float x, float y);
    void        goto_origin(float x, float y);
    void        goto_takepic2_pos(float pcb_width, float pcb_height);
    void        set_takepic2_calib(void);
    void        goto_takepic2_calib(void);
    void        start_scan(void);

    void        stop_scan(void);
    void        goto_home_pos(void);
    void        ask_for_camera_height(void);
    void        allow_emit_area(bool);

    void        set_scan_parameters(double);
    void        scan_heigh_update(double);
    void        tool_change(int, int, int);
    void        Z_shift(double);
    void        X_shift(double);
    void        Y_shift(double);
    void        probe_rotate();

    void        calib_x_enable(bool);
    void        calib_y_enable(bool);

    inline void assign_rs_instrument(RS_Instruments *i)       { ins = i;                        };
    inline void assign_duts(DUT_size *p, DUT_size *s)         { pcb = p; scan_area = s;         };
    inline void assign_logfile(Event_log *lf)                 { log_file = lf;                  };
    inline void set_scan_components(bool x, bool y)           { x_comp_sel = x; y_comp_sel = y; };

    RS_Instruments *ins;
    quint64 total_time = 0;


    double height_off_x=0.0, height_off_y=0.0, start_scan_off_x=0.0, start_scan_off_y=0.0;

private slots:
     void read_msg(void);
     void robotBytesWritten(qint64 b) { Q_UNUSED(b); };

signals:
    void robot_msg_to_terminal(QString);
    void scan_finished(void);
    void allow_emit_pos(bool);
    void stop_displaying_point(void);
    void height_measured(void);
    void inst_thread_init(void);
    void send_area_to_videothread(void);
    void send_real_pcb_height(uint32_t);
    void save_cropped_image(void);
    void scan_error(void);

private:

    const QString robot_ip_address = "192.168.11.2";
    const int robot_port = 23;

    DUT_size *pcb;
    DUT_size *scan_area;

    Event_log *log_file;
    QByteArray robot_raw_data;
    bool scan_started = false;
    bool robot_first_run = true;
    bool y_comp = true;
    bool emit_area = false;
    bool start_scan_error = false;
    bool probe_rotation = false;

    bool x_comp_sel{false};
    bool y_comp_sel{false};

    bool x_axis{true};
    bool switch_arrows{false};

    bool first_scan_height_update{true};
    uint8_t cnt = 0;
};

#endif // ROBOT_H
