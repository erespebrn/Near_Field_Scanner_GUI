#ifndef RS_VNA_H
#define RS_VNA_H

#include "rs_instruments.h"
#include <QTimer>
#include <memory>

class RS_vna : public RS_Instruments
{
public:
    RS_vna(void);

    virtual bool init(void)                  override;
    virtual void request_freq_values(void)   override;
    virtual void request_magnitudes(void)    override;
    virtual void request_duration_time(void) override;
    virtual void send_command(QString &)     override;
    virtual void send_command(const char*)   override;
    virtual void save_data_to_file(Save)     override;


private slots:
    void dataread(void) override;
    void confirm_written_bytes(qint64 bytes);
    void check_for_mag_received(void);
    
private:
    const QString vna_ip_addr{"192.168.11.6"};
    const unsigned int vna_port{5025};

    bool vna_stimulus_values = false;


    void request_mag(void);
    void request_phase(void);

    std::unique_ptr<QTimer> timer{};

};

#endif // RS_VNA_H
