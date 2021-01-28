#ifndef RS_INSTRUMENTS_H
#define RS_INSTRUMENTS_H

#include <QObject>
#include <QTcpSocket>
#include <QDate>
#include <QDir>

#include "event_log.h"

class RS_Instruments : public QObject
{
    Q_OBJECT
public:
    enum Instrument {VNA,SA};

    RS_Instruments(Instrument, QString);
    ~RS_Instruments();

    bool tcp_connect();
    void init_scan();
    void request_freq_values();
    void request_magnitudes();
    void request_duration_time();
    void update_current_position(uint16_t x, uint16_t y);
    void save_data_to_file(char);
    void reset_saved_data();
    void get_datapath(QString dp);
    QTcpSocket* give_socket();

    //Scan rows and columns (max. x and y position)
    uint16_t scan_rows = 0;
    uint16_t scan_columns = 0;

    //Scan step size
    uint16_t step_size = 0;

    //Sweep points ans no. of sweeps
    int32_t sweep_points;
    uint8_t no_of_sweeps = 1;

    //Single sweep duration time (read from the instrument)
    double dur_time = 2.0;

    //Log file
    Event_log *log_file;

    //Device name from the scan wizard
    QString dut_name;

private slots:
    void sa_dataread();
    void vna_dataread();
    void confirm_written_bytes(qint64);

signals:
    void stop_scan();

private:
    //Enum to choose SA or VNA
    Instrument ins;

    //Current position on the scan
    uint16_t x_pos = 0;
    uint16_t y_pos = 0;

    //Tcp socket
    QTcpSocket *socket;
    QString ip_address = "192.168.11.4";

    //Buffor data array and check bytes variable
    QByteArray b_data;
    int32_t bytes = 0;

    //Flags
    bool duration_time = false;
    bool time_for_amplitude = false;
    bool vna_stimulus_values = false;
    bool scan_started = false;

    //Data storage vectors
    std::vector<std::vector<std::vector<float>>> data_tensor;
    std::vector<std::vector<float>> temp2d;
    std::vector<float> freq;

    //Datapath string
    QString current_scan_datapath;

};

#endif // RS_INSTRUMENTS_H
