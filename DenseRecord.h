#ifndef DENSERECORD_H
#define DENSERECORD_H
#include <vector>
#include <utility>
#include <cmath>
#include "DataRecord.h"

/**
 *  Class that derives from the DataRecord class. It defines characteristic features of the dense data type and gives recipe for distance calculation
 *  between two dense data records.
 */

class DenseRecord : public DataRecord
{
public:
    DenseRecord();
    DenseRecord(std::vector<float> p, float d, int i);
    std::vector<float> parameters;
    float calculateDistance(DenseRecord reference_record, int p);
    bool operator==(DenseRecord p);
    friend std::ostream & operator<<(std::ostream & os, const DenseRecord &obj);
};

#endif // DENSERECORD_H
