#include "Typedef.h"

class DataTypeConverter{
	public:
	DenseDataVector sparseToDense(SparseDataVector inputVector, int dimension);
	SparseDataVector denseToSparse(DenseDataVector inputVector);
	
};
