#include "rs_instr_detector_thread.h"

Instrument_Thread::Instrument_Thread(const QString sa_ip_address, const QString vna_ip_address) : sa_ip{sa_ip_address}, vna_ip{vna_ip_address}
{}

Instrument_Thread::~Instrument_Thread()
{}

void Instrument_Thread::start()
{
    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &Instrument_Thread::scan_devices);
    timer->start(1000);
}

void Instrument_Thread::scan_devices()
{
    QTcpSocket *_vna_socket = new QTcpSocket;
    QTcpSocket *_sa_socket = new QTcpSocket;

    bool vna_online;
    bool sa_online;
    QString msg = "";

    if(_vna_socket->state() == QAbstractSocket::UnconnectedState)
    {
        _vna_socket->connectToHost(vna_ip, 5025);
        _vna_socket->waitForConnected(10);

    }
    if(_sa_socket->state() == QAbstractSocket::UnconnectedState)
    {
        _sa_socket->connectToHost(sa_ip, 5025);
        _sa_socket->waitForConnected(10);
    }

    if(_vna_socket->state() == QAbstractSocket::ConnectedState)
    {
        vna_online = true;
        _vna_socket->write("SYST:TSL OFF\n");
        _vna_socket->waitForBytesWritten();
        msg = "";

    }
    else
        vna_online = false;

    if(_sa_socket->state() == QAbstractSocket::ConnectedState)
        sa_online = true;
    else
        sa_online = false;

    delete _sa_socket;
    delete _vna_socket;
    emit VNA_connected(vna_online);
    emit SA_connected(sa_online);
}

void Instrument_Thread::shutdown()
{
    qDebug() << "Instrument thread shutdown..";
    emit finished();
}

