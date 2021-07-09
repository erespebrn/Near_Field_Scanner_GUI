#ifndef TCP_DEVICE_H
#define TCP_DEVICE_H

#include <QTcpSocket>

class Tcp_device : public QTcpSocket
{
    Q_OBJECT
public:
    Tcp_device();
    bool tcp_connect(const QString& ip_addr, const unsigned int port);
};

#endif // TCP_DEVICE_H
