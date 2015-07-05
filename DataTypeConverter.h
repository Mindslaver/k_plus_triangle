#include "Typedef.h"

/**
 *  Class used for the data records' type conversions
 */

namespace DataTypeConverter
{
    DenseDataVector sparseToDense(SparseDataVector inputVector, int dimension);
    SparseDataVector denseToSparse(DenseDataVector inputVector);
}
