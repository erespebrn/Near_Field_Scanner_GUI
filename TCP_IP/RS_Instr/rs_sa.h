#ifndef RS_SA_H
#define RS_SA_H

#include "rs_instruments.h"

class RS_sa : public RS_Instruments
{
public:
    RS_sa(void);

    virtual bool init(void)                  override;
    virtual void request_freq_values(void)   override;
    virtual void request_magnitudes(void)    override;
    virtual void request_duration_time(void) override;
    virtual void send_command(QString&)      override;
    virtual void send_command(const char*)   override;
    virtual void save_data_to_file(Save)     override;

private slots:
    void dataread(void) override;
    void confirm_written_bytes(qint64 bytes);

private:
    const QString sa_ip_address{"192.168.11.4"};
    const unsigned int sa_port{5025};
};

#endif // RS_SA_H
