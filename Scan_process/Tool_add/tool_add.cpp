#include "tool_add.h"
#include "ui_tool_add.h"
#include "scanner_gui.h"

#include <QPushButton>
#include <QLineEdit>
#include <QVector>
#include <QString>
#include <QListWidget>
#include <QLabel>

tool_add::tool_add(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::tool_add)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle("Tool Settings");
    tools_init();
}

tool_add::~tool_add()
{
    tools_save();
    emit tool_tab_closed(Tools);
    delete ui;
}

void tool_add::on_Add_Tool_clicked()
{
    QString T_Name = ui->Tool_Name_val->text();
    bool Tool_In_List = false;

    for (int i = 0; i < Tools.length(); i++) {
        if (Tools[i]->tool_name == T_Name){
            Tool_In_List = !Tool_In_List;
        }
    }

    if (!Tool_In_List && T_Name != "" && ui->X_Off_val->text() != "" && ui->Y_Off_val->text() != "" && ui->Z_Off_val->text() != ""){
        float X_off = ui->X_Off_val->text().toFloat();
        float Y_off = ui->Y_Off_val->text().toFloat();
        float Z_off = ui->Z_Off_val->text().toFloat();
        Tool *newTool = new Tool(T_Name,X_off,Y_off,Z_off);
        Tools.append(newTool);
        ///qDebug() << T_Name;
        ///qDebug() << X_off;
        ///qDebug() << Y_off;
        ///qDebug() << Z_off;
        ui->Tool_List->addItem(T_Name);

        Clear_Fields();
    }
    else if (Tool_In_List) {
        QMessageBox::critical(this, "Error!", "Duplicate Tool!");
    }
    else {
        QMessageBox::critical(this, "Error!", "Please fill in all the fields!");
    }

}

void tool_add::tools_init(){
    ui->Tool_Picture->setPixmap(QPixmap(QCoreApplication::applicationDirPath() + "/Add_Probe_Illustration_Final.jpg"));

    QDoubleValidator* X_val= new QDoubleValidator(this);
    X_val->setNotation(QDoubleValidator::StandardNotation);
    ui->X_Off_val->setValidator(X_val);

    QDoubleValidator* Y_val= new QDoubleValidator(this);
    Y_val->setNotation(QDoubleValidator::StandardNotation);
    ui->Y_Off_val->setValidator(Y_val);

    QDoubleValidator* Z_val= new QDoubleValidator(this);
    Z_val->setNotation(QDoubleValidator::StandardNotation);
    ui->Z_Off_val->setValidator(Z_val);

    ///read tools from the txt file
    QFile inputFile(QCoreApplication::applicationDirPath() + "/tooldata.txt");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList fields = line.split(";");
            Tool* T = new Tool(fields[0],fields[1].toFloat(),fields[2].toFloat(),fields[3].toFloat());
            ///qDebug() << fields;
            Tools.append(T);
        }
        inputFile.close();
    }

    for (int i = 0; i < Tools.length(); i++) {
        ui->Tool_List->addItem(Tools[i]->tool_name);
    }
}

void tool_add::on_Exit_clicked()
{
    //tools_save();
    //emit tool_tab_closed(Tools);
    close();
}

void tool_add::on_Remove_Tool_clicked()
{
    if (ui->Tool_List->currentItem() != NULL ){

        QString T_Name = ui->Tool_List->currentItem()->text();
        ///qDebug() << T_Name;

        for (int i = 0; i < Tools.length(); i++) {
            if (Tools[i]->tool_name == T_Name){
                ///qDebug() << Tools[i]->tool_name << Tools[i]->tool_x << Tools[i]->tool_y << Tools[i]->tool_z;
                Tools.remove(i);
            }
        }

        delete ui->Tool_List->currentItem();
        Clear_Fields();
    }
}

void tool_add::tools_save(){
    QFile file(QCoreApplication::applicationDirPath() + "/tooldata.txt");
    file.remove();

    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
        QTextStream stream(&file);

        for (int i = 0; i < Tools.length(); i++) {
            stream << Tools[i]->tool_name << ";" << Tools[i]->tool_x << ";" << Tools[i]->tool_y << ";" << Tools[i]->tool_z << "\n";
            }

        file.close();
        }
}

void tool_add::on_pushButton_clicked()
{
    ///qDebug() << "modify clicked";
    if (ui->Tool_List->currentItem() != NULL ){
        for (int i = 0; i < Tools.length(); i++) {
            if (Tools[i]->tool_name == ui->Tool_List->currentItem()->text()){
                ///qDebug() << ui->Tool_List->currentItem()->text();

                delete ui->Tool_List->currentItem();

                Tools[i]->tool_name = ui->Tool_Name_val->text();
                Tools[i]->tool_x = ui->X_Off_val->text().toFloat();
                Tools[i]->tool_y = ui->Y_Off_val->text().toFloat();
                Tools[i]->tool_z = ui->Z_Off_val->text().toFloat();

                ui->Tool_List->addItem(Tools[i]->tool_name);

                Clear_Fields();
                /*ui->Tool_Name_val->setText(Tools[i]->tool_name);
                ui->X_Off_val->setText(QString::number((Tools[i]->tool_x)));
                ui->Y_Off_val->setText(QString::number((Tools[i]->tool_y)));
                ui->Z_Off_val->setText(QString::number((Tools[i]->tool_z)));*/
            }
            if (Tools[i]->tool_name == ""){
                Tools.remove(i);
                delete ui->Tool_List->item(i);
            }
        }
    }
}

void tool_add::on_Tool_List_itemClicked(QListWidgetItem *item)
{
    ///Q_UNUSED(item);
    ///qDebug() << item->text();
    ///qDebug() << "modify clicked";
    if (item->text() != NULL ){
        for (int i = 0; i < Tools.length(); i++) {
            if (Tools[i]->tool_name == item->text()){
                ///qDebug() << ui->Tool_List->currentItem()->text();
                ui->Tool_Name_val->setText(Tools[i]->tool_name);
                ui->X_Off_val->setText(QString::number((Tools[i]->tool_x)));
                ui->Y_Off_val->setText(QString::number((Tools[i]->tool_y)));
                ui->Z_Off_val->setText(QString::number((Tools[i]->tool_z)));
            }
        }
    }
}

void tool_add::Clear_Fields(){
    ui->Tool_Name_val->setText("");
    ui->X_Off_val->setText("");
    ui->Y_Off_val->setText("");
    ui->Z_Off_val->setText("");
}

void tool_add::mouseReleaseEvent(QMouseEvent *ev){
    Q_UNUSED(ev);
    for (int i = 0; i < Tools.length(); i++) {
        ui->Tool_List->item(i)->setSelected(false);
    }
    Clear_Fields();
}
