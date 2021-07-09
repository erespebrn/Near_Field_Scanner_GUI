#include "tcp_device.h"

Tcp_device::Tcp_device() : QTcpSocket{}
{}

bool Tcp_device::tcp_connect(const QString &ip_addr, const unsigned int port)
{
    bool success{false};

    this->QTcpSocket::connectToHost(ip_addr, port);
    this->QTcpSocket::waitForConnected(10);

    (this->QTcpSocket::state() == QAbstractSocket::ConnectedState) ? success = true : success = false;
    return success;
}

