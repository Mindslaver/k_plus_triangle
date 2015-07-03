#include "DataTypeConverter.h"
#include <iostream>

using namespace std;

DenseDataVector DataTypeConverter::sparseToDense(SparseDataVector inputVector, int dimension)
{
	DenseDataVector denseVector;
	denseVector.resize(inputVector.size());
	int i = 0;
	for(SparseDataVector::iterator it = inputVector.begin(); it != inputVector.end(); ++it){
		denseVector[i].resize(dimension);
        for(vector<pair<int,float>>::iterator rowIt = (*it).begin(); rowIt != (*it).end();++rowIt){
			denseVector[i][(*rowIt).first] = (*rowIt).second;
		}
		++i;
	}
	return denseVector;
}

SparseDataVector DataTypeConverter::denseToSparse(DenseDataVector inputVector)
{
    SparseDataVector sparseVector;
    for(DenseDataVector::iterator it = inputVector.begin(); it != inputVector.end(); ++it)
    {
        vector<pair<int,float>> tmpVector;
        int i = 1;
        for(vector<float>::iterator it2 = (*it).begin(); it2 != (*it).end();++it2)
        {
            tmpVector.push_back(make_pair(i,(float) *it2));
        ++i;
        }
        sparseVector.push_back(tmpVector);
    }
    return sparseVector;
}

