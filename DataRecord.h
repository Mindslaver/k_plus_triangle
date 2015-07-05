#ifndef DATARECORD_H
#define DATARECORD_H
#include <iostream>

/**
 *  The base class for the data records which can be either sparse or dense.
 *  Dense record has parameters, number of which is equal to the number of the all unique parameters of the all records in the dataset.
 *  Sparse record contains only these parameters that are explicitly specified for this record.
 */

class DataRecord
{
public:
    DataRecord();

    //Distance between a given data record and an object that will be classified
    float distance;
    //Epsilon distance, prediction of the actual distance, which can be smaller but not higher
    float eps;
    int id;
    //Position on the sorted list of candidades for k neighbours with respect to eps
    int position;

    //Overloads of operators for comparisons
    bool operator<(DataRecord r);
    bool operator>(DataRecord r);
    bool operator<(float r);
    bool operator>(float r);
    friend bool operator<(float p, const DataRecord & pt);
    friend bool operator<(const DataRecord & pt1, const DataRecord & pt2);

    //Overload of substraction useful for calculating the distance
    float operator-(DataRecord r);

    //Prints out an id of a given data record enclosed by "<>"
    friend std::ostream & operator<<=(std::ostream & os, const DataRecord &obj);

};

#endif // DATARECORD_H
