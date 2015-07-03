#ifndef SPARSERECORD_H
#define SPARSERECORD_H
#include <vector>
#include <utility>
#include <cmath>
#include "DataRecord.h"

class SparseRecord : public DataRecord
{
public:
    SparseRecord();
    SparseRecord(std::vector<std::pair<int, float>> p, float d, int i);
    std::vector<std::pair<int, float>> parameters;
    float calculateDistance(SparseRecord reference_point, int p);
    bool operator==(SparseRecord p);
    friend std::ostream & operator<<(std::ostream & os, const SparseRecord &obj);
};

#endif
