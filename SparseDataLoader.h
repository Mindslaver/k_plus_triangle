#ifndef SPARSEDATALOADER_H
#define SPARSEDATALOADER_H
#include "Typedef.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

/**
 *  Class used to load and store sparse data
 */

class SparseDataLoader
{
	public:
    SparseDataLoader():maxIndex(0)
    {
    }
	
    void readSparseData(std::string filename);
    SparseDataVector getRowVector();
	void displayRow();
    int maxIndex;

    SparseDataVector rowVector;
};

#endif
