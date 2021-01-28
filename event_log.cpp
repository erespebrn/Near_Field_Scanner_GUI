#include "event_log.h"

Event_log::Event_log()
{}

Event_log::~Event_log()
{}

void Event_log::log_file_init(QString dut_name)
{
    qDebug() << "DUT name: " << dut_name;
    file = new std::fstream;
    QString path = QCoreApplication::applicationDirPath() + "/event_logs/" + QDate::currentDate().toString("dd_MM_yyyy") + "__" + QTime::currentTime().toString("hh_mm_ss") + "_" + dut_name + ".txt";
    file->open(path.toLocal8Bit(),std::ios_base::out);
}

void Event_log::add_log_line(QString line)
{
    if(file->is_open()){
        line = line + "\n";
        const char *l = line.toLocal8Bit();
        file->write(l, strlen(l));
    }
    else
        qDebug() << "File not open";
}

void Event_log::save_to_new_logfile()
{
    file->flush();
    file->close();
    delete file;
}

QStringList Event_log::load_logfile_list()
{
    QDir *directory = new QDir(QCoreApplication::applicationDirPath() + "/event_logs/");
    QStringList files = directory->entryList(QStringList() << "*.txt", QDir::Files);
    delete directory;
    return files;
}
