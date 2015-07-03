#ifndef DATARECORD_H
#define DATARECORD_H
#include <iostream>

class DataRecord
{
private:
public:
    DataRecord();
    float distance;
    float eps;
    int id;
    int position;
    bool operator<(DataRecord r);
    bool operator>(DataRecord r);
    bool operator<(float r);
    bool operator>(float r);
    float operator-(DataRecord r);
    friend std::ostream & operator<<=(std::ostream & os, const DataRecord &obj);
    friend bool operator<(float p, const DataRecord & pt);
    friend bool operator<(const DataRecord & pt1, const DataRecord & pt2);
};

#endif // DATARECORD_H
