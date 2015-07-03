#include "Typedef.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <stdexcept>

class DenseDataLoader{
	public:
    DenseDataLoader():maxIndex(0){}
	
    void readDenseData(std::string filename);
    DenseDataVector getDataVector()
    {
		return denseDataVector;
	}
	
	void displayData();
	
    DenseDataVector denseDataVector;
	double maxIndex;
	
};
