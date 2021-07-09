#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <QString>
#include <QDate>
#include <QTime>
#include <fstream>
#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QFile>

class Event_log
{

public:
    enum Log_type { events, scan };
    Event_log(Log_type t);
    ~Event_log();
    void log_file_init(const QString&);
    void add_log_line(QString);
    void save_to_new_logfile();
    QStringList load_logfile_list();

private:
    std::fstream *file;
    Log_type type;
    QSettings *scan_log;
};

#endif // EVENT_LOG_H
