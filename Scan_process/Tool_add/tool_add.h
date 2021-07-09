#ifndef TOOL_H
#define TOOL_H
#include <QString>


class Tool
{
public:
    Tool(QString name, float x, float y, float z)
    {
        tool_name = name;
        tool_x = x;
        tool_y = y;
        tool_z = z;
    };
    ~Tool();
    QString tool_name;
    float tool_x;
    float tool_y;
    float tool_z;
};

#endif // TOOL_H


#ifndef TOOL_ADD_H
#define TOOL_ADD_H
#include "scanner_gui.h"

#include <QPushButton>
#include <QLineEdit>
#include <QVector>
#include <QWidget>
#include <QDialog>
#include <QListWidget>

namespace Ui {
class tool_add;
}

class tool_add : public QDialog
{
    Q_OBJECT

public:
    explicit tool_add(QWidget *parent = nullptr);
    ~tool_add();

signals:
    void tool_tab_closed(QVector<Tool*>);

private slots:
    void on_pushButton_clicked();
    void on_Add_Tool_clicked();
    void on_Exit_clicked();

    void on_Remove_Tool_clicked();

    void on_Tool_List_itemClicked(QListWidgetItem *item);
    ///This might not work
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    void tools_init();
    void Clear_Fields();
    void tools_save();

    Ui::tool_add *ui;
    QVector<Tool*> Tools;
};

#endif // TOOL_ADD_H

