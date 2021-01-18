#ifndef RS_INSTRUMENTS_H
#define RS_INSTRUMENTS_H

#include <QObject>
#include <QTcpSocket>

class RS_Instruments : public QObject
{
    Q_OBJECT
public:
    enum Instrument {VNA,SA};

    RS_Instruments(Instrument);
    ~RS_Instruments();
    void request_freq_values();
    void request_magnitudes();
    void update_current_position(uint16_t x, uint16_t y);
    void get_sweep_points(int);
    void save_data_to_file(char);
    void reset_saved_data();
    bool tcp_connect(QString);
    QTcpSocket* give_socket();

    uint16_t scan_rows = 0;
    uint16_t scan_columns = 0;
private slots:
    void sa_dataread();
    void confirm_written_bytes(qint64);

signals:
    void stop_scan();
private:
    Instrument ins;
    uint16_t x_pos = 0;
    uint16_t y_pos = 0;

    QTcpSocket *socket;

    const QString sa_ip_address = "192.168.11.4";
    const QString vna_ip_address = "192.168.11.6";

    QByteArray b_data;

    int32_t bytes = 0;
    int32_t sweep_points;

    bool time_for_amplitude = false;

    std::vector<std::vector<std::vector<float>>> data_tensor;
    std::vector<std::vector<float>> temp2d;
    std::vector<float> freq;

    QString datapath = "C:/Users/Near-field scanner/Documents/Near_Field_Scanner_GUI/datastorage/scan_data/";
    QString foldername = "SCAN_21_10_2020__15_34_11";
    QString current_scan_datapath;


};

#endif // RS_INSTRUMENTS_H
