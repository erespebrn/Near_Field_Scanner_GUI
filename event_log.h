#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <QString>
#include <QDate>
#include <QTime>
#include <fstream>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

class Event_log
{

public:
    Event_log();
    ~Event_log();
    void log_file_init(QString);
    void add_log_line(QString);
    void save_to_new_logfile();
    QStringList load_logfile_list();

private:
    std::fstream *file;
};

#endif // EVENT_LOG_H
