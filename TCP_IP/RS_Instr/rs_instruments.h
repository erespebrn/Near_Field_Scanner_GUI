#ifndef RS_INSTRUMENTS_H
#define RS_INSTRUMENTS_H

#include <QObject>
#include <QTcpSocket>
#include <QDate>
#include <QDir>
#include <memory>
#include <QTcpSocket>

#include "Scan_process/Plotting/nf_plot.h"
#include "Scan_process/Event_log/event_log.h"

#include "TCP_IP/tcp_device.h"

class RS_Instruments : public Tcp_device
{
    Q_OBJECT
public:
    enum Save {FREQ, XCOMP, YCOMP};
    enum VNA_mode{Mode_A, Mode_B};
    enum RS_ins_type{SA, VNA};

    RS_Instruments();
    ~RS_Instruments();

    //Pure virtual functions. The interface for SA/VNA classes.
    virtual bool init(void)                  = 0;
    virtual void send_command(QString &)     = 0;
    virtual void send_command(const char*)   = 0;
    virtual void request_freq_values(void)   = 0;
    virtual void request_magnitudes(void)    = 0;
    virtual void request_duration_time(void) = 0;
    virtual void save_data_to_file(Save)     = 0;


    void set_vna_mode(VNA_mode m) { mode = m; };

    void init_scan(void);
    void reset_saved_data(void);

    inline void     update_current_position(uint16_t x, uint16_t y) { x_pos = x; y_pos = y;               };
    inline void     set_datapath(QString dp)                        { current_scan_datapath = dp;         };
    inline uint16_t get_scan_rows(void) const                       { return scan_rows;                   };
    inline void     set_scan_rows(const uint16_t &value)            { scan_rows = value;                  };
    inline uint16_t get_scan_columns(void) const                    { return scan_columns;                };
    inline void     set_scan_columns(const uint16_t &value)         { scan_columns = value;               };
    inline double   get_duration_time(void) const                   { return dur_time;                    };
    inline void     set_duration_time(const double &value)          { dur_time = value;                   };
    inline void     set_nearfield_plot(NF_plot *nf)                 { nearfield_plot = nf;                };
    inline void     set_eventlog_file(Event_log * el)               { log_file = el;                      };
    inline void     set_step_size(const uint16_t &value)            { step_size = value;                  };
    inline void     set_sweep_points(const uint32_t &value)         { sweep_points = value;               };
    inline void     set_no_of_sweeps(const uint8_t &value)          { no_of_sweeps = value;               };
    inline void     allow_nearfield_plot_exports(bool a)            { nearfield_plot->allow_exports(a);   };
    inline void     set_nearfield_plot_current_scan_comp
                    (NF_plot::Current_scan_comp c)                  { nearfield_plot->set_current_scan_comp(c); };

    inline RS_ins_type& get_rs_ins_type(void) { return rs_type; };

private slots:
    virtual void dataread() = 0;

signals:
    void stop_scan();

protected:
    //Data storage vectors
    std::vector<std::vector<std::vector<float>>> data_tensor;
    std::vector<std::vector<std::vector<float>>> data_tensor_mem;
    std::vector<std::vector<float>>              temp2d;
    std::vector<std::vector<float>>              temp2d_phase;
    std::vector<float>                           freq;
    std::vector<float>                           mag;
    std::vector<float>                           phase;
    std::vector<std::vector<std::vector<float>>> data_tensor_phase;
    std::vector<std::vector<std::vector<float>>> data_tensor_mem_phase;

    //Buffor data array and check bytes variable
    QByteArray b_data{};
    int32_t bytes{0};

    //Current position on the scan
    uint16_t x_pos{0};
    uint16_t y_pos{0};

    //Flags
    bool time_for_amplitude{false};
    bool vna_stimulus_values{false};
    bool scan_started{false};

    //Log file
    Event_log *log_file{nullptr};

    //Scan rows and columns (max. x and y position)
    uint16_t scan_rows{0};
    uint16_t scan_columns{0};

    //Sweep points and no. of sweeps
    int32_t sweep_points{0};
    uint8_t no_of_sweeps{1};

    //Single sweep duration time (read from the instrument)
    double dur_time{2.0};
    bool duration_time{false};

    //Device name from the scan wizard
    QString dut_name{};

    //Scan step size
    uint16_t step_size{0};

    //Datapath string
    QString current_scan_datapath{};

    //Plotting tool
    NF_plot *nearfield_plot{};

    VNA_mode mode{};

    bool mag_received{false};
    RS_ins_type rs_type{};

private:




};

#endif // RS_INSTRUMENTS_H
