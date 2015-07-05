#include "SparseRecord.h"

using namespace std;

SparseRecord::SparseRecord()
{
}

SparseRecord::SparseRecord(vector<pair<int, float>> p, float d, int i) : parameters(p)
{
    distance=d;
    id=i;
}

float SparseRecord::calculateDistance(SparseRecord reference_record, int p)
{
    float sum = 0;

    int dimensionT = parameters.size();
    int dimensionB = reference_record.parameters.size();
    int top=0;
    int bot=0;
    bool skipBot=false;
    bool skipTop=false;
    while(1)
    {
        if(parameters[top].first>reference_record.parameters[bot].first)
        {
            sum += fabs(pow(reference_record.parameters[bot].second, p));
            if(bot+1!=dimensionB)
                ++bot;
            else
                skipBot=true;
        }
        else if(parameters[top].first<reference_record.parameters[bot].first)
        {
            sum += fabs(pow(parameters[top].second, p));
            if(top+1!=dimensionT)
                ++top;
            else
                skipTop=true;
        }
        else
        {
            sum += fabs(pow(parameters[top].second - reference_record.parameters[bot].second, p));
            if(top+1!=dimensionT)
                ++top;
            else
                skipTop=true;
            if(bot+1!=dimensionB)
                ++bot;
            else
                skipBot=true;
        }
        if(skipBot&&skipTop)
        {
            return pow(sum, 1.0 / p);
        }
        if(skipBot)
        {
            while(1)
            {
                sum += fabs(pow(parameters[top].second, p));
                if(top+1!=dimensionT)
                    ++top;
                else
                    return pow(sum, 1.0 / p);
            }
        }
        if(skipTop)
        {
            while(1)
            {
                sum += fabs(pow(reference_record.parameters[bot].second, p));
                if(bot+1!=dimensionB)
                    ++bot;
                else
                    return pow(sum, 1.0 / p);
            }
        }
    }
}

bool SparseRecord::operator==(SparseRecord p)
{
    if ((*this).parameters == p.parameters)
    {
        return true;
    } else
        return false;
}

ostream & operator<<(ostream & os, const SparseRecord &obj)
{
    if(obj.id>-1)
        os << "<" << obj.id << "> ";
    os << "[";
    int i = obj.parameters.size();
    for (int j = 0; j < i; j++) {
        os << "("<< obj.parameters[j].first << ": " << obj.parameters[j].second;
        if (j < i)
            os << ")";
    }
    os << "]";
    return os;
}
