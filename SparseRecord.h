#ifndef SPARSERECORD_H
#define SPARSERECORD_H
#include <vector>
#include <utility>
#include <cmath>
#include "DataRecord.h"

/**
 *  Class that derives from the DataRecord class. It defines characteristic features of the sparse data type and gives recipe for distance calculation
 *  between two sparse data records.
 */

class SparseRecord : public DataRecord
{
public:
    SparseRecord();
    SparseRecord(std::vector<std::pair<int, float>> p, float d, int i);
    std::vector<std::pair<int, float>> parameters;
    float calculateDistance(SparseRecord reference_record, int p);
    bool operator==(SparseRecord p);
    friend std::ostream & operator<<(std::ostream & os, const SparseRecord &obj);
};

#endif
