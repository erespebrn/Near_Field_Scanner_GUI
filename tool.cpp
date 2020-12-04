#include "tool.h"

Tool::Tool(QString name,float x,float y,float z){
    tool_name = name;
    tool_x = x;
    tool_y = y;
    tool_z = z;
}

Tool::~Tool()
{
}
