#include "DenseRecord.h"

using namespace std;

DenseRecord::DenseRecord()
{
}

DenseRecord::DenseRecord(vector<float> p, float d, int i) : parameters(p)
{
    distance=d;
    id=i;
}

float DenseRecord::calculateDistance(DenseRecord reference_record, int p)
{
    float sum = 0;
    int dimensions = reference_record.parameters.size();
    for (int i = 0; i < dimensions; i++)
    {
        sum += fabs(pow((parameters[i] - reference_record.parameters[i]), p));
    }
    return pow(sum, 1.0 / p);
}

bool DenseRecord::operator==(DenseRecord p) {
    if ((*this).parameters == p.parameters) {
        return true;
    } else
        return false;
}

ostream & operator<<(ostream & os, const DenseRecord &obj) {
    if(obj.id>-1)
        os << "<" << obj.id << "> ";
    os << "[";
    int i = obj.parameters.size();
    for (int j = 0; j < i; j++) {
        os << obj.parameters[j];
        if (j < i - 1)
            os << ",";
    }
    os << "]";
    return os;
}
