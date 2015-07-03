#ifndef DENSERECORD_H
#define DENSERECORD_H
#include <vector>
#include <utility>
#include <cmath>
#include "DataRecord.h"

class DenseRecord : public DataRecord
{
public:
    DenseRecord();
    DenseRecord(std::vector<float> p, float d, int i);
    std::vector<float> parameters;
    float calculateDistance(DenseRecord reference_point, int p);
    bool operator==(DenseRecord p);
    friend std::ostream & operator<<(std::ostream & os, const DenseRecord &obj);
};

#endif // DENSERECORD_H
