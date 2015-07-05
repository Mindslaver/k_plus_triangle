#ifndef DENSEDATALOADER_H
#define DENSEDATALOADER_H
#include "Typedef.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <stdexcept>

/**
 *  Class used to load and store dense data
 */

class DenseDataLoader
{
	public:
    DenseDataLoader():maxIndex(0)
    {
    }
	
    void readDenseData(std::string filename);
    void displayData();
    DenseDataVector getDataVector();

    DenseDataVector denseDataVector;
	double maxIndex;
};

#endif
