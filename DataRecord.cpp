#include "DataRecord.h"

using namespace std;

DataRecord::DataRecord()
{
}

bool DataRecord::operator<(DataRecord p)
{
    if ((*this).distance < p.distance)
        return true;
    else
        return false;
}
bool DataRecord::operator>(DataRecord p)
{
    if ((*this).distance > p.distance)
        return true;
    else
        return false;
}
bool DataRecord::operator<(float p)
{
    if ((*this).distance < p)
        return true;
    else
        return false;
}
bool DataRecord::operator>(float p)
{
    if ((*this).distance > p)
        return true;
    else
        return false;
}
float DataRecord::operator-(DataRecord p)
{
    return (*this).distance - p.distance;
}

ostream & operator<<=(ostream & os, const DataRecord &obj)
{
    os << "<" << obj.id << ">";
    return os;
}

bool operator<(float p, const DataRecord & pt)
{
    return p < pt.distance;
}

bool operator<(const DataRecord & pt1, const DataRecord & pt2)
{
    return pt1.distance < pt2.distance;
}
