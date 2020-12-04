#ifndef TOOL_H
#define TOOL_H
#include <QString>


class Tool
{
public:
    Tool(QString name,float  x,float y,float z);
    ~Tool();
    QString tool_name;
    float tool_x;
    float tool_y;
    float tool_z;
};

#endif // TOOL_H
