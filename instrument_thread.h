#ifndef INSTRUMENT_THREAD_H
#define INSTRUMENT_THREAD_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QMessageBox>
#include <QHostInfo>

class Instrument_Thread : public QObject {
    Q_OBJECT
public:
    Instrument_Thread(const QString, const QString);
    ~Instrument_Thread();

public slots:
    void start();
    void scan_devices();
    void shutdown();

signals:
    void SA_connected(bool);
    void VNA_connected(bool);
    void finished();

private:
    QTimer * timer;
    QTcpSocket * _vna_socket;
    QTcpSocket * _sa_socket;
    QHostInfo * info;

    QString sa_ip = "";
    QString vna_ip = "";

};

#endif // INSTRUMENT_THREAD_H
