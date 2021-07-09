#include "event_log.h"

Event_log::Event_log(Log_type t) : type {t}
{}

Event_log::~Event_log()
{}

void Event_log::log_file_init(const QString& dut_name)
{
    if(type == events){
        file = new std::fstream;
        QString path = QCoreApplication::applicationDirPath() + "/event_logs/" + QDate::currentDate().toString("dd_MM_yyyy") + "__" + QTime::currentTime().toString("hh_mm_ss") + "_" + dut_name + ".txt";
        file->open(path.toLocal8Bit(),std::ios_base::out);
    }else{
        QDir *dir = new QDir(QCoreApplication::applicationDirPath() + "/scan_logs/");
        QStringList dirs = dir->entryList(QDir::Dirs|QDir::NoDotAndDotDot);
        QString f_path = dut_name + " " + QDate::currentDate().toString("dd_MM_yyyy") + "_" + QTime::currentTime().toString("hh_mm_ss");
        dir->mkdir(dut_name + " " + QDate::currentDate().toString("dd_MM_yyyy") + "_" + QTime::currentTime().toString("hh_mm_ss"));
        delete dir;
        QFile::copy(QCoreApplication::applicationDirPath() + "/settings/" + "scansettings.ini", QCoreApplication::applicationDirPath() + "/scan_logs/"+ f_path + "/scansettings.ini");
    }

}

void Event_log::add_log_line(QString line)
{
    Q_UNUSED(line);
//    if(type == events){
//        if(file->is_open()){
//            line = line + "\n";
//            const char *l = line.toLocal8Bit();
//            file->write(l, strlen(l));
//        }else{
//            qDebug() << "Log file not open. ERROR!" << line;
//    }
//    }else{

//    }
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
